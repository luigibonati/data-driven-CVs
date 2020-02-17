/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   Copyright (c) 2011-2018 The plumed team
   (see the PEOPLE file at the root of the distribution for a list of names)

   See http://www.plumed.org for more information.

   This file is part of plumed, version 2.

   plumed is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   plumed is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with plumed.  If not, see <http://www.gnu.org/licenses/>.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
#include "Function.h"
#include "ActionRegister.h"

#include <torch/torch.h>
#include <torch/script.h>

#include <cmath>

using namespace std;

std::vector<float> tensor_to_vector(const torch::Tensor& x) {
    return std::vector<float>(x.data<float>(), x.data<float>() + x.numel());
}

namespace PLMD {
namespace function {

//+PLUMEDOC FUNCTION AUTOENCODER
//+ENDPLUMEDOC


class PytorchModel :
  public Function
{
  unsigned _n_in;
  unsigned _n_out;
  std::shared_ptr<torch::jit::script::Module> _model;
  float mean_, range_;
public:
  explicit PytorchModel(const ActionOptions&);
  void calculate();
  static void registerKeywords(Keywords& keys);
};


PLUMED_REGISTER_ACTION(PytorchModel,"PYTORCH_MODEL")

void PytorchModel::registerKeywords(Keywords& keys) {
  Function::registerKeywords(keys);
  keys.use("ARG");
  keys.add("optional","MODEL","filename of the trained model"); 
  keys.add("optional","MIN","min of the inputs");
  keys.add("optional","MAX","max of the inputs"); 
}

PytorchModel::PytorchModel(const ActionOptions&ao):
  Action(ao),
  Function(ao)
  //powers(getNumberOfArguments(),1.0)
{
  _n_in=getNumberOfArguments();

  std::string fname="model.pt";
  parse("MODEL",fname);  

  //std::ifstream in(fname, std::ios_base::binary);

  _model = torch::jit::load(fname); //or in
  assert(_model != nullptr);

  float min=-1.,max=1.;
  parse("MIN",min);
  parse("MAX",max);
  mean_=(max+min)/2.;
  range_=(max-min)/2.;

  checkRead();

  //check the dimension of the output
  log.printf("Checking input dimension:\n");
  std::vector<float> input_test (_n_in);
  torch::Tensor single_input = torch::tensor(input_test).view({1,_n_in});  
  std::vector<torch::jit::IValue> inputs;
  inputs.push_back( single_input );
  auto output = _model->forward( inputs ).toTensor();
  vector<float> cvs = tensor_to_vector (output);
  _n_out=cvs.size();

  //log.printf("  without periodic boundary conditions\n");
  for(unsigned j=0; j<_n_out; j++){
    addComponentWithDerivatives( std::to_string(j) ); 
    componentIsNotPeriodic( std::to_string(j) );
  }

  log.printf("Pytorch Model Loaded: %s\n",fname);
  log.printf("Number of input: %d\n",_n_in); 
  log.printf("Number of outputs: %d\n",_n_out); 

}

void PytorchModel::calculate() {

  //retrieve arguments
  vector<float> current_S(_n_in);
  for(unsigned i=0; i<_n_in; i++)
    current_S[i]=(getArgument(i)-mean_)/range_;
  //convert to tensor
  torch::Tensor input_S = torch::tensor(current_S).view({1,_n_in});
  input_S.set_requires_grad(true);
  //convert to Ivalue
  std::vector<torch::jit::IValue> inputs;
  inputs.push_back( input_S );
  //encode
  auto output = _model->forward( inputs ).toTensor();
  output.backward();

  vector<float> cvs = tensor_to_vector (output);
  vector<float> der = tensor_to_vector (input_S.grad() );

  for(unsigned i=0; i<_n_in; i++)
    setDerivative(i,der[i]);

  for(unsigned j=0; j<_n_out; j++)
    getPntrToComponent(std::to_string(j))->set(cvs[j]);
  
/*
  double combine=0.0;
  for(unsigned i=0; i<coefficients.size(); ++i) {
    double cv = (getArgument(i)-parameters[i]);
    combine+=coefficients[i]*pow(cv,powers[i]);
    setDerivative(i,coefficients[i]*powers[i]*pow(cv,powers[i]-1.0));
  };
  setValue(combine);
*/
}

}
}


