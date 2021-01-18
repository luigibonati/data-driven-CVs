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
#include "core/Colvar.h"
#include "core/ActionRegister.h"
#include "tools/Pbc.h"

#include <string>
#include <cmath>

#include <numeric> 
#include <sstream>
#include <iterator>

#include "tools/Tools.h"
#include "tools/SwitchingFunction.h"
#include <string>

using namespace std;

namespace PLMD {
namespace colvar {


class Contacts : public Colvar {
  bool components;
  bool reorderon;
  bool pbc;
  SwitchingFunction switchingFunction;
  bool twogroups;

public:
  static void registerKeywords( Keywords& keys );
  explicit Contacts(const ActionOptions&);
//active methods:
  virtual void calculate();
  vector<AtomNumber> atomsa;
  vector<AtomNumber> atomsb;
  int num_atomsa;
  int num_atomsb;
  int num_dist;
};

PLUMED_REGISTER_ACTION(Contacts,"CONTACTS")

void Contacts::registerKeywords( Keywords& keys ) {
  Colvar::registerKeywords( keys );
  keys.addFlag("REORDER",false,"reorder the list of contacts");  
  keys.addFlag("COMPONENTS",true,"calculate the x, y and z components of the Contacts separately and store them as label.x, label.y and label.z");
  keys.add("atoms","GROUPA","First list of atoms");
  keys.add("atoms","GROUPB","Second list of atoms (if empty, N*(N-1)/2 pairs in GROUPA are counted)");
  keys.add("compulsory","NN","6","The n parameter of the switching function ");
  keys.add("compulsory","MM","0","The m parameter of the switching function; 0 implies 2*NN");
  keys.add("compulsory","D_0","0.0","The d_0 parameter of the switching function");
  keys.add("compulsory","R_0","The r_0 parameter of the switching function");
  keys.add("optional","SWITCH","This keyword is used if you want to employ an alternative to the continuous swiching function defined above. "
                               "The following provides information on the \\ref switchingfunction that are available. "
                               "When this keyword is present you no longer need the NN, MM, D_0 and R_0 keywords.");
  keys.addOutputComponent("c", "default", "contacts");
}

Contacts::Contacts(const ActionOptions&ao):
  PLUMED_COLVAR_INIT(ao),
  components(true),
  reorderon(false),
  pbc(true),
  twogroups(true)
{
  parseAtomList("GROUPA",atomsa);
  parseAtomList("GROUPB",atomsb);
  num_atomsa = atomsa.size();
  num_atomsb = atomsb.size(); 

  vector<AtomNumber> atoms;
  for(unsigned int i=0; i<num_atomsa; i++)
  {
    AtomNumber ata;
    ata.setIndex(atomsa[i].index());
    atoms.push_back(ata);
  }
  for(unsigned int i=0; i<num_atomsb; i++)
  {
    AtomNumber atb;
    atb.setIndex(atomsb[i].index());
    atoms.push_back(atb);
  }

  parseFlag("COMPONENTS",components);
  bool nopbc=!pbc;
  parseFlag("NOPBC",nopbc);
  pbc=!nopbc;
  //log.printf("%d\n\n\n\n",reorderon);
  parseFlag("REORDER",reorderon);
  //log.printf("%d\n\n\n\n",reorderon);

  //addValueWithDerivatives(); setNotPeriodic();
  if(num_atomsb>0){
    num_dist = num_atomsa*num_atomsb;
  } else {
    num_dist = (num_atomsa*(num_atomsa-1))/2;
    twogroups = false;
  }

  log.printf("  between two groups of %u and %u atoms\n",static_cast<unsigned>(num_atomsa),static_cast<unsigned>(num_atomsb));
  log.printf("  first group:\n");
  for(unsigned int i=0;i<num_atomsa;++i){
   if ( (i+1) % 25 == 0 ) log.printf("  \n");
   log.printf("  %d", atomsa[i].serial());
  }
  log.printf("  \n  second group:\n");
  for(unsigned int i=0;i<num_atomsb;++i){
   if ( (i+1) % 25 == 0 ) log.printf("  \n");
   log.printf("  %d", atomsb[i].serial());
  }
  log.printf("  \n");

  //log.printf("  between atoms %d %d\n",atoms[0].serial(),atoms[1].serial());
  if(pbc) log.printf("  using periodic boundary conditions\n");
  else    log.printf("  without periodic boundary conditions\n");

  // Read in the switching function
  std::string sw, errors; parse("SWITCH",sw);
  if(sw.length()>0){
     switchingFunction.set(sw,errors);
     if( errors.length()!=0 ) error("problem reading SWITCH keyword : " + errors );
  } else {
     double r_0=-1.0, d_0; int nn, mm;
     parse("NN",nn); parse("MM",mm);
     parse("R_0",r_0); parse("D_0",d_0);
     if( r_0<0.0 ) error("you must set a value for R_0");
     switchingFunction.set(nn,mm,r_0,d_0);
  }

  //log.printf("  Tetrahedral order parameter calculated computing the angles with the atoms within %s\n",( switchingFunction.description() ).c_str() );

  if(components) {
     for(int i=0; i<num_dist; i++)
     {
        auto label = "c-"+std::to_string(i);
        addComponentWithDerivatives(label); componentIsNotPeriodic(label);
     }
  }

  requestAtoms(atoms);
  checkRead();

}


// calculator
void Contacts::calculate() {

  if(pbc) makeWhole();

  vector<Vector> atom_pos=getPositions();
  int N_atoms = atom_pos.size();

  std::vector<Vector> dist(num_dist); 
  std::vector<double> invdist(num_dist); 
  std::vector<double> distmod(num_dist); 
  double sw, df, df2;				//switching functions and derivatives

  int ind = 0;
  if(twogroups) {
      for(int i=0; i<num_atomsa; i++)
      {
         for(int j=0; j<num_atomsb; j++)
         {
            if(pbc) dist[ind]=pbcDistance(getPosition(i),getPosition(j+num_atomsa));
            else dist[ind]=delta(getPosition(i),getPosition(j+num_atomsa));
            invdist[ind]=1.0/dist[ind].modulo();
            distmod[ind]=dist[ind].modulo();
            ind++;
         }
      }
  } else{
      for(int i=0; i<N_atoms; i++)
      {
         for(int j=i+1; j<N_atoms; j++)
         {
            if(pbc) dist[ind]=pbcDistance(getPosition(i),getPosition(j));
            else dist[ind]=delta(getPosition(i),getPosition(j));
            invdist[ind]=1.0/dist[ind].modulo();
            distmod[ind]=dist[ind].modulo();
            ind++;
         }
      }
  }

  //ORDERING the vector
  //indices will contain the labels of the old vector reordered, I care about the position in the new vector though
  std::vector<int> indices(num_dist); 
  std::iota(begin(indices), end(indices), 0);
  std::sort(begin(indices), end(indices),
          [&distmod](int lhs, int rhs) { return distmod[lhs] < distmod[rhs]; });
  //std::copy(begin(indices), end(indices), std::ostream_iterator<int>(std::cout, " "));
  std::vector<int> indicesnew(num_dist); 
  for(int k=0; k<num_dist; k++)
  {
    indicesnew[indices[k]] = k;
  }

  /*
  std::array<double,4> A = {6.5, 2.2, 5.6, 7.1};
  std::array<int,4> indices;
  std::iota(begin(indices), end(indices), 0); // initialize the index array
  // sort with custom comparator, that compares items not by their own value
  // but by the value of the corresponding entries in the original array.
  std::sort(begin(indices), end(indices),
          [&A](int lhs, int rhs) { return A[lhs] < A[rhs]; });
  std::copy(begin(indices), end(indices), std::ostream_iterator<int>(std::cout, " "));
  */

  if(components) {
     int ind = 0;
     if(twogroups) {
         for(int i=0; i<num_atomsa; i++)
         {
            for(int j=0; j<num_atomsb; j++)
            {
               //SWITCHING function, it takes the square of the dist and gives sw. Its derivative/dist is df
               sw = switchingFunction.calculateSqr( pow(dist[ind].modulo(),2), df );
               df2 = df*dist[ind].modulo();

               //make it go from -1 to 1 for the NN
               sw = (2.0*sw)-1.0;
               df2 = 2.0*df2;

               //auto label = std::to_string(ind);               //NON REORDERED
               //if(reorderon) auto label = std::to_string(indicesnew[ind]);   //REORDERED
               //auto label = std::to_string(indicesnew[ind]);   //REORDERED
               if(reorderon){
                  auto label = "c-"+std::to_string(indicesnew[ind]);   //REORDERED

                  Value* val=getPntrToComponent(label);
                  setAtomsDerivatives (val,i,-invdist[ind]*dist[ind]*df2);
                  setAtomsDerivatives (val,j+num_atomsa,invdist[ind]*dist[ind]*df2);
                  setBoxDerivativesNoPbc(val);
                  val->set(sw);
               }
               else{ 
                  auto label = "c-"+std::to_string(ind);               //NON REORDERED

                  Value* val=getPntrToComponent(label);
                  setAtomsDerivatives (val,i,-invdist[ind]*dist[ind]*df2);
                  setAtomsDerivatives (val,j+num_atomsa,invdist[ind]*dist[ind]*df2);
                  setBoxDerivativesNoPbc(val);
                  val->set(sw);
               }        

               ind = ind+1;
            }
         }
     } else{
         for(int i=0; i<N_atoms; i++)
         {
            for(int j=i+1; j<N_atoms; j++)
            {
               //SWITCHING function, it takes the square of the dist and gives sw. Its derivative/dist is df
               sw = switchingFunction.calculateSqr( pow(dist[ind].modulo(),2), df );
               df2 = df*dist[ind].modulo();

               //make it go from -1 to 1 for the NN
               sw = (2.0*sw)-1.0;
               df2 = 2.0*df2;

               //auto label = std::to_string(ind);               //NON REORDERED
               //if(reorderon) auto label = std::to_string(indicesnew[ind]);   //REORDERED
               //auto label = std::to_string(indicesnew[ind]);   //REORDERED
               if(reorderon){
                  auto label = "c-"+std::to_string(indicesnew[ind]);   //REORDERED

                  Value* val=getPntrToComponent(label);
                  setAtomsDerivatives (val,i,-invdist[ind]*dist[ind]*df2);
                  setAtomsDerivatives (val,j,invdist[ind]*dist[ind]*df2);
                  setBoxDerivativesNoPbc(val);
                  val->set(sw);
               }
               else{ 
                  auto label = "c-"+std::to_string(ind);               //NON REORDERED

                  Value* val=getPntrToComponent(label);
                  setAtomsDerivatives (val,i,-invdist[ind]*dist[ind]*df2);
                  setAtomsDerivatives (val,j,invdist[ind]*dist[ind]*df2);
                  setBoxDerivativesNoPbc(val);
                  val->set(sw);
               }        

               ind = ind+1;
            }

         }
     }
  }

}

}
}



