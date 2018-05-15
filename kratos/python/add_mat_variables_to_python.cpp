//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License 
//                   Kratos default license: kratos/license.txt
//
//  Main authors:  Josep Maria Carbonell
//


// System includes

// External includes


// Project includes
#include "includes/define_python.h"
#include "includes/variables.h"
#include "includes/mat_variables.h"
#include "python/add_mat_variables_to_python.h"

//commented variables are registered in add_containers_to_python.cpp

namespace Kratos
{

  namespace Python
  {
    using namespace pybind11;

    void  AddMATVariablesToPython(pybind11::module& m)
    {
    //registering variables in python

      //solution
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CONSTITUTIVE_LAW_NAME )

      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, IMPLEX_CONTACT )
      //elasticity
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, YOUNG_MODULUS )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, POISSON_RATIO )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHEAR_MODULUS_GAMMA12)
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHEAR_MODULUS_GAMMA12_2)
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHEAR_MODULUS_GAMMA12_3)
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHEAR_MODULUS_GAMMA12_4)
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DENSITY )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, THICKNESS )
      //KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, EQUIVALENT_YOUNG_MODULUS )
      //KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BULK_MODULUS )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHEAR_MODULUS )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, LAME_MU )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, LAME_LAMBDA )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, C10 )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, C20 )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, C30 )

      //viscosity
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, VISCOSITY )

      //damage
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DAMAGE_VARIABLE )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DAMAGE_THRESHOLD )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, STRENGTH_RATIO )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FRACTURE_ENERGY )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RESIDUAL_STRENGTH )

      //plasticity
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PLASTIC_STRAIN )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DELTA_PLASTIC_STRAIN )      
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, NORM_ISOCHORIC_STRESS )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PLASTIC_STRAIN_RATE )

      //hardening
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ISOTROPIC_HARDENING_MODULUS )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, KINEMATIC_HARDENING_MODULUS )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, HARDENING_EXPONENT )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, REFERENCE_HARDENING_MODULUS )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, INFINITY_HARDENING_MODULUS )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SOFTENING_SLOPE )

      //baker-johnson-cook parameters
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, JC_PARAMETER_A )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, JC_PARAMETER_B )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, JC_PARAMETER_C )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, JC_PARAMETER_m )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, JC_PARAMETER_n )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, JC_PARAMETER_K ) 

      //thermal
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DELTA_PLASTIC_STRAIN )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, REFERENCE_CONDUCTIVITY )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, HARDNESS_CONDUCTIVITY )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, REFERENCE_TEMPERATURE )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MELD_TEMPERATURE )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PLASTIC_DISSIPATION )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DELTA_PLASTIC_DISSIPATION )

      //anisotropy
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, YOUNG_MODULUS_X )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, YOUNG_MODULUS_Y )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, YOUNG_MODULUS_Z )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHEAR_MODULUS_XY )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHEAR_MODULUS_YZ )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHEAR_MODULUS_XZ )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, POISSON_RATIO_XY )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, POISSON_RATIO_YZ )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, POISSON_RATIO_XZ )

      KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, MATERIAL_ORIENTATION_DX )
      KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, MATERIAL_ORIENTATION_DY )
      KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, MATERIAL_ORIENTATION_DZ )

      //critical state
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CRITICAL_STATE_LINE )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PRE_CONSOLIDATION_STRESS )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, OVER_CONSOLIDATION_RATIO )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, INITIAL_SHEAR_MODULUS )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, NORMAL_COMPRESSION_SLOPE )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SWELLING_SLOPE )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ALPHA_SHEAR )

      //strain
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DETERMINANT_F )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DEFORMATION_GRADIENT )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, GREEN_LAGRANGE_STRAIN_VECTOR )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, GREEN_LAGRANGE_STRAIN_TENSOR )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ALMANSI_STRAIN_VECTOR )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ALMANSI_STRAIN_TENSOR )

      //stress
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, KIRCHHOFF_STRESS_VECTOR )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, KIRCHHOFF_STRESS_TENSOR )

      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CAUCHY_STRESS_VECTOR )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CAUCHY_STRESS_TENSOR )

      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PK2_STRESS_VECTOR )
      KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PK2_STRESS_TENSOR )
    }
  }  // namespace Python.
} // Namespace Kratos


