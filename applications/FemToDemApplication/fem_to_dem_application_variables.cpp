//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics FemDem Application
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Alejandro Cornejo Velázquez
//

#include "fem_to_dem_application_variables.h"

namespace Kratos
{
	KRATOS_CREATE_VARIABLE(double, DAMAGE_EDGE1)
	KRATOS_CREATE_VARIABLE(double, DAMAGE_EDGE2)
	KRATOS_CREATE_VARIABLE(double, DAMAGE_EDGE3)
	KRATOS_CREATE_VARIABLE(double, DAMAGE_ELEMENT)
	KRATOS_CREATE_VARIABLE(Vector,STRESS_VECTOR)
	KRATOS_CREATE_VARIABLE(double, YIELD_STRESS_C)
	KRATOS_CREATE_VARIABLE(double, YIELD_STRESS_T)
	KRATOS_CREATE_VARIABLE(int, ITER)
	KRATOS_CREATE_VARIABLE(double, FRAC_ENERGY_T)
	KRATOS_CREATE_VARIABLE(double, FRAC_ENERGY_C)
	KRATOS_CREATE_VARIABLE(Vector, STRESS_VECTOR_INTEGRATED)
	KRATOS_CREATE_VARIABLE(double, THRESHOLD)
	KRATOS_CREATE_VARIABLE(Vector, SMOOTHED_STRESS_VECTOR)
	KRATOS_CREATE_VARIABLE(std::string,YIELD_SURFACE)
	KRATOS_CREATE_VARIABLE(Vector, STRAIN_VECTOR)
	KRATOS_CREATE_VARIABLE(bool, TANGENT_CONSTITUTIVE_TENSOR)
	KRATOS_CREATE_VARIABLE(bool, SMOOTHING)
	KRATOS_CREATE_VARIABLE(double, IS_DAMAGED)
	KRATOS_CREATE_VARIABLE(double, NODAL_DAMAGE)
	//KRATOS_CREATE_VARIABLE(double, CHARACTERISTIC_LENGTH)
	KRATOS_CREATE_VARIABLE(int, MESH_REFINED)
	KRATOS_CREATE_VARIABLE(int, IS_DYNAMIC)
	KRATOS_CREATE_VARIABLE(double, STRESS_THRESHOLD)
	KRATOS_CREATE_VARIABLE(double, INITIAL_THRESHOLD)
	KRATOS_CREATE_VARIABLE(int, INTEGRATION_COEFFICIENT)
	KRATOS_CREATE_VARIABLE(std::string, MAPPING_PROCEDURE)
	KRATOS_CREATE_VARIABLE(bool, IS_DEM)
	KRATOS_CREATE_VARIABLE(double, DEM_RADIUS)
	KRATOS_CREATE_VARIABLE(bool, DEM_GENERATED)
	KRATOS_CREATE_VARIABLE(bool, INACTIVE_NODE)
	KRATOS_CREATE_VARIABLE(int, NUMBER_OF_ACTIVE_ELEMENTS)
	KRATOS_CREATE_VARIABLE(bool, NODAL_FORCE_APPLIED)
	KRATOS_CREATE_VARIABLE(double, NODAL_FORCE_X)
	KRATOS_CREATE_VARIABLE(double, NODAL_FORCE_Y)
	KRATOS_CREATE_VARIABLE(double, NODAL_FORCE_Z)
	KRATOS_CREATE_VARIABLE(Vector, NODAL_STRESS_VECTOR)
	KRATOS_CREATE_VARIABLE(double, EQUIVALENT_NODAL_STRESS)
	KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS(EQUIVALENT_NODAL_STRESS_GRADIENT)
	KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS(AUXILIAR_GRADIENT)

	KRATOS_CREATE_VARIABLE(Matrix, STRAIN_TENSOR);
	KRATOS_CREATE_VARIABLE(Matrix, STRESS_TENSOR);
	KRATOS_CREATE_VARIABLE(Matrix, STRESS_TENSOR_INTEGRATED);
	
	// Composite
	KRATOS_CREATE_VARIABLE(Matrix, CONCRETE_STRESS_TENSOR);
	KRATOS_CREATE_VARIABLE(Matrix, STEEL_STRESS_TENSOR);
	KRATOS_CREATE_VARIABLE(Vector, CONCRETE_STRESS_VECTOR);
	KRATOS_CREATE_VARIABLE(Vector, STEEL_STRESS_VECTOR);
	KRATOS_CREATE_VARIABLE(double,YOUNG_MODULUS_STEEL);
	KRATOS_CREATE_VARIABLE(double,DENSITY_STEEL);
	KRATOS_CREATE_VARIABLE(double,POISSON_RATIO_STEEL);
	KRATOS_CREATE_VARIABLE(double,STEEL_VOLUMETRIC_PART);
	KRATOS_CREATE_VARIABLE(Matrix,CONCRETE_STRESS_TENSOR_INTEGRATED);
	
	KRATOS_CREATE_VARIABLE(double,YIELD_STRESS_C_STEEL);
	KRATOS_CREATE_VARIABLE(double,YIELD_STRESS_T_STEEL);
	KRATOS_CREATE_VARIABLE(double,FRACTURE_ENERGY_STEEL);
	KRATOS_CREATE_VARIABLE(double,PLASTIC_DISSIPATION_CAPAP);
	KRATOS_CREATE_VARIABLE(double,EQUIVALENT_STRESS_VM);
	KRATOS_CREATE_VARIABLE(int,HARDENING_LAW);
	KRATOS_CREATE_VARIABLE(double,MAXIMUM_STRESS);
	KRATOS_CREATE_VARIABLE(double,MAXIMUM_STRESS_POSITION);
	
	KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS(LINE_LOAD);
	KRATOS_CREATE_3D_VARIABLE_WITH_COMPONENTS(SURFACE_LOAD);
}
