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

#include "includes/define.h"
#include <string>
#include "includes/constitutive_law.h"
#include "custom_constitutive/zarate_law.hpp"
#include "femdem2d_element.hpp"
#include "includes/element.h"
#include "fem_to_dem_application_variables.h"
#include "includes/kratos_flags.h"
#include "containers/flags.h"
#include "solid_mechanics_application_variables.h"

namespace Kratos
{
//***********************DEFAULT CONSTRUCTOR******************************************
//************************************************************************************

FemDem2DElement::FemDem2DElement(IndexType NewId, GeometryType::Pointer pGeometry)
	: SmallDisplacementElement(NewId, pGeometry)
{
	//DO NOT ADD DOFS HERE!!!
}
//******************************CONSTRUCTOR*******************************************
//************************************************************************************

FemDem2DElement::FemDem2DElement(IndexType NewId, GeometryType::Pointer pGeometry, PropertiesType::Pointer pProperties)
	: SmallDisplacementElement(NewId, pGeometry, pProperties)
{
	//BY DEFAULT, THE GEOMETRY WILL DEFINE THE INTEGRATION METHOD
	mThisIntegrationMethod = GetGeometry().GetDefaultIntegrationMethod();
}

//******************************COPY CONSTRUCTOR**************************************
//************************************************************************************

FemDem2DElement::FemDem2DElement(FemDem2DElement const &rOther)
	: SmallDisplacementElement(rOther)
{
	//ALL MEMBER VARIABLES THAT MUST BE KEPT AFTER COPYING AN ELEMENT HAVE TO BE DEFINED HERE
	//IF NO ASSIGMENT OPERATOR IS DEFINED THE COPY CONSTRUCTOR WILL DEFINE IT BY DEFFAULT
}

//*******************************ASSIGMENT OPERATOR***********************************
//************************************************************************************

FemDem2DElement &FemDem2DElement::operator=(FemDem2DElement const &rOther)
{
	//ALL MEMBER VARIABLES THAT MUST BE KEPT IN AN "=" OPERATION NEEDS TO BE COPIED HERE

	SmallDisplacementElement::operator=(rOther);
	return *this;
}

//*********************************OPERATIONS*****************************************
//************************************************************************************

Element::Pointer FemDem2DElement::Create(IndexType NewId, NodesArrayType const &rThisNodes, PropertiesType::Pointer pProperties) const
{
	//NEEDED TO CREATE AN ELEMENT
	return Element::Pointer(new FemDem2DElement(NewId, GetGeometry().Create(rThisNodes), pProperties));
}

//************************************CLONE*******************************************
//************************************************************************************

Element::Pointer FemDem2DElement::Clone(IndexType NewId, NodesArrayType const &rThisNodes) const
{

	//YOU CREATE A NEW ELEMENT CLONING THEIR VARIABLES
	//ALL MEMBER VARIABLES THAT MUST BE CLONED HAVE TO BE DEFINED HERE

	FemDem2DElement NewElement(NewId, GetGeometry().Create(rThisNodes), pGetProperties());

	return Element::Pointer(new FemDem2DElement(NewElement));
}

//*******************************DESTRUCTOR*******************************************
//************************************************************************************

FemDem2DElement::~FemDem2DElement()
{
}

void FemDem2DElement::InitializeSolutionStep(ProcessInfo &rCurrentProcessInfo)
{

	// After the mapping, the thresholds of the edges ( are equal to 0.0) are imposed equal to the IP threshold
	Vector thresholds = this->GetThresholds();
	double ElementThreshold = this->GetValue(STRESS_THRESHOLD);

	if (thresholds[0] == 0.0 && thresholds[1] == 0.0 && thresholds[2] == 0.0)
	{
		this->SetThreshold(ElementThreshold, 0);
		this->SetThreshold(ElementThreshold, 1);
		this->SetThreshold(ElementThreshold, 2);
	}

	// IDEM with the edge damages
	Vector DamageEdges = this->GetDamages();
	double DamageElement = this->GetValue(DAMAGE_ELEMENT);

	if (DamageEdges[0] == 0.0 && DamageEdges[1] == 0.0 && DamageEdges[2] == 0.0)
	{
		this->SetConvergedDamages(DamageElement, 0);
		this->SetConvergedDamages(DamageElement, 1);
		this->SetConvergedDamages(DamageElement, 2);
	}
}

void FemDem2DElement::FinalizeSolutionStep(ProcessInfo &rCurrentProcessInfo)
{
	double current_equivalent_stress = 0.0, damage_element = 0.0;

	//Loop over edges
	for (int cont = 0; cont < 3; cont++)
	{
		this->SetConvergedDamages(this->GetNonConvergedDamages(cont), cont);
		this->SetConvergedEquivalentStresses(this->GetNonConvergedEquivalentStress(cont), cont);
		current_equivalent_stress = this->GetConvergedEquivalentStresses(cont);
		if (current_equivalent_stress > this->GetThreshold(cont))
		{
			this->SetThreshold(current_equivalent_stress, cont);
		}
	} // End Loop over edges

	damage_element = this->Get_NonConvergeddamage();
	this->SetConvergedDamage(damage_element);

	// if (damage_element > 0.0) {
	// 	this->SetValue(IS_DAMAGED, 1);
	// }

	if (damage_element >= 0.98)
	{
		this->Set(ACTIVE, false);
		double old_threshold = this->GetValue(STRESS_THRESHOLD);
		this->SetValue(INITIAL_THRESHOLD, old_threshold);
	}

	this->ResetNonConvergedVars();
	this->SetToZeroIteration();

	// computation of the equivalent damage threshold and damage of the element for AMR mapping
	Vector thresholds = this->GetThresholds();

	Vector two_min_values;
	this->Get2MaxValues(two_min_values, thresholds[0], thresholds[1], thresholds[2]); // todo ojo con la funcion modificada
	double equivalent_threshold = 0.5 * (two_min_values[0] + two_min_values[1]);	  // El menor o mayor?? TODO

	this->SetValue(STRESS_THRESHOLD, equivalent_threshold); // AMR
	this->SetThreshold(equivalent_threshold);
	this->SetValue(DAMAGE_ELEMENT, damage_element);

	// Reset the nodal force flag for the next time step
	Geometry<Node<3>> &nodes_element = this->GetGeometry();
	for (int i = 0; i < 3; i++)
	{
		#pragma omp critical
		{
			nodes_element[i].SetValue(NODAL_FORCE_APPLIED, false);
		}
	}
}

void FemDem2DElement::InitializeNonLinearIteration(ProcessInfo &rCurrentProcessInfo)
{
	//*****************************
	KRATOS_TRY

	bool is_active = true;
	if (this->IsDefined(ACTIVE))
	{
		is_active = this->Is(ACTIVE);
	}

	// Inactive elements can have negative determinant of the Jacobian
	if (is_active == true)
	{
		//1.-Initialize sizes for the system components:
		const unsigned int number_of_nodes = GetGeometry().size();
		const unsigned int dimension = GetGeometry().WorkingSpaceDimension();
		unsigned int voigt_size = dimension * (dimension + 1) * 0.5;

		Vector StrainVector(voigt_size);
		noalias(StrainVector) = ZeroVector(voigt_size);
		Vector StressVector(voigt_size);
		noalias(StressVector) = ZeroVector(voigt_size);
		Matrix ConstitutiveMatrix(voigt_size, voigt_size);
		noalias(ConstitutiveMatrix) = ZeroMatrix(voigt_size, voigt_size);
		Matrix B(voigt_size, dimension * number_of_nodes);
		noalias(B) = ZeroMatrix(voigt_size, dimension * number_of_nodes);
		Matrix DN_DX(number_of_nodes, dimension);
		noalias(DN_DX) = ZeroMatrix(number_of_nodes, dimension);

		//deffault values for the infinitessimal theory
		double detF = 1;
		Matrix F(dimension, dimension);
		noalias(F) = identity_matrix<double>(dimension);

		//3.-Calculate elemental system:

		//reading integration points
		const GeometryType::IntegrationPointsArrayType &integration_points = GetGeometry().IntegrationPoints(mThisIntegrationMethod);

		//get the shape functions [N] (for the order of the default integration method)
		const Matrix &Ncontainer = GetGeometry().ShapeFunctionsValues(mThisIntegrationMethod);

		//get the shape functions parent coodinates derivative [dN/d�] (for the order of the default integration method)
		const GeometryType::ShapeFunctionsGradientsType &DN_De = GetGeometry().ShapeFunctionsLocalGradients(mThisIntegrationMethod);

		//calculate delta position (here coincides with the current displacement)
		Matrix DeltaPosition(number_of_nodes, dimension);
		noalias(DeltaPosition) = ZeroMatrix(number_of_nodes, dimension);
		DeltaPosition = this->CalculateDeltaPosition(DeltaPosition);

		//calculating the reference jacobian from cartesian coordinates to parent coordinates for all integration points [dx_n/d�]
		GeometryType::JacobiansType J;
		J.resize(1, false);
		J[0].resize(dimension, dimension, false);
		noalias(J[0]) = ZeroMatrix(dimension, dimension);
		J = GetGeometry().Jacobian(J, mThisIntegrationMethod, DeltaPosition);

		// Loop Over Integration Points
		for (unsigned int PointNumber = 0; PointNumber < integration_points.size(); PointNumber++)
		{
			Matrix InvJ(dimension, dimension);
			noalias(InvJ) = ZeroMatrix(dimension, dimension);
			double detJ = 0;
			MathUtils<double>::InvertMatrix(J[PointNumber], InvJ, detJ);

			if (detJ < 0)
			{
				this->Set(ACTIVE, false); // element alone inside a crack
				detJ = fabs(detJ);
			}

			if (detJ < 0)
				KRATOS_THROW_ERROR(std::invalid_argument, " SMALL DISPLACEMENT ELEMENT INVERTED: |J|<0 ) detJ = ", detJ)

			//compute cartesian derivatives for this integration point  [dN/dx_n]
			noalias(DN_DX) = prod(DN_De[PointNumber], InvJ);

			//set shape functions for this integration point
			Vector N = row(Ncontainer, PointNumber);

			//b.-compute infinitessimal strain
			this->CalculateInfinitesimalStrain(StrainVector, DN_DX);
			//this->SetStrainVector(StrainVector);
			this->SetValue(STRAIN_VECTOR, StrainVector);

			ConstitutiveLaw::Parameters Values(GetGeometry(), GetProperties(), rCurrentProcessInfo);

			//set constitutive law variables: (it passes only references to this local variables)
			Values.SetStrainVector(StrainVector);
			Values.SetStressVector(StressVector);
			Values.SetConstitutiveMatrix(ConstitutiveMatrix);
			Values.SetShapeFunctionsDerivatives(DN_DX);
			Values.SetShapeFunctionsValues(N);
			//values to be set:
			Values.SetDeterminantF(detF);
			Values.SetDeformationGradientF(F);

			//set constitutive law flags:
			Flags &ConstitutiveLawOptions = Values.GetOptions();

			//compute stress and constitutive matrix
			ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_STRESS);
			ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR);

			//CALL THE CONSTITUTIVE LAW (for this integration point)
			//(after calling the constitutive law StressVector and ConstitutiveMatrix are set and can be used)
			mConstitutiveLawVector[PointNumber]->CalculateMaterialResponseCauchy(Values);
			this->SetValue(STRESS_VECTOR, Values.GetStressVector());

			this->CalculateDeformationMatrix(B, DN_DX);
			this->SetBMatrix(B);
		}
	}
	KRATOS_CATCH("")
}

void FemDem2DElement::CalculateLocalSystem(MatrixType &rLeftHandSideMatrix, VectorType &rRightHandSideVector, ProcessInfo &rCurrentProcessInfo)
{
	KRATOS_TRY

	const unsigned int number_of_nodes = GetGeometry().size();
	const unsigned int dimension = GetGeometry().WorkingSpaceDimension();
	unsigned int voigt_size = dimension * (dimension + 1) / 2;

	const GeometryType::IntegrationPointsArrayType &integration_points = GetGeometry().IntegrationPoints(mThisIntegrationMethod);
	unsigned int system_size = number_of_nodes * dimension;
	if (rLeftHandSideMatrix.size1() != system_size)
		rLeftHandSideMatrix.resize(system_size, system_size, false);
	noalias(rLeftHandSideMatrix) = ZeroMatrix(system_size, system_size);

	if (rRightHandSideVector.size() != system_size)
		rRightHandSideVector.resize(system_size, false);
	noalias(rRightHandSideVector) = ZeroVector(system_size);

	Matrix DeltaPosition(number_of_nodes, dimension);
	noalias(DeltaPosition) = ZeroMatrix(number_of_nodes, dimension);
	DeltaPosition = this->CalculateDeltaPosition(DeltaPosition);

	GeometryType::JacobiansType J;
	J.resize(1, false);
	J[0].resize(dimension, dimension, false);
	noalias(J[0]) = ZeroMatrix(dimension, dimension);
	J = GetGeometry().Jacobian(J, mThisIntegrationMethod, DeltaPosition);

	for (unsigned int PointNumber = 0; PointNumber < integration_points.size(); PointNumber++)
	{
		const Matrix &Ncontainer = GetGeometry().ShapeFunctionsValues(mThisIntegrationMethod);
		Vector N = row(Ncontainer, PointNumber);
		double detJ = 0.0;
		Matrix InvJ(dimension, dimension);
		noalias(InvJ) = ZeroMatrix(dimension, dimension);
		MathUtils<double>::InvertMatrix(J[PointNumber], InvJ, detJ);

		double IntegrationWeight = integration_points[PointNumber].Weight() * detJ;
		if (dimension == 2)
			IntegrationWeight *= GetProperties()[THICKNESS];
		this->SetValue(INTEGRATION_COEFFICIENT, IntegrationWeight);
		const Matrix &B = this->GetBMatrix();

		Vector IntegratedStressVector = ZeroVector(voigt_size);

		// Find Neighbour Elements
		WeakPointerVector<Element> &elem_neigb = this->GetValue(NEIGHBOUR_ELEMENTS);
		if (elem_neigb.size() == 0)
		{
			KRATOS_THROW_ERROR(std::invalid_argument, " Neighbour Elements not calculated --> size = ", elem_neigb.size())
		}

		// Compute damage on each edge of the element
		double damage[3] = {0.0, 0.0, 0.0};

		// Loop Over Edges
		for (int cont = 0; cont < 3; cont++)
		{
			bool is_active_neigh = true;
			if (elem_neigb[cont].IsDefined(ACTIVE))
			{
				is_active_neigh = elem_neigb[cont].Is(ACTIVE);
			}
			double damagee = 0.0;
			Vector AverageStress;
			Vector AverageStrain;

			if (is_active_neigh)
			{
				Vector Stress1, Stress2;
				Vector Strain1, Strain2;

				Stress1 = this->GetValue(STRESS_VECTOR);
				Stress2 = elem_neigb[cont].GetValue(STRESS_VECTOR);

				Strain1 = this->GetValue(STRAIN_VECTOR);
				Strain2 = elem_neigb[cont].GetValue(STRAIN_VECTOR);

				this->AverageVector(AverageStress, Stress1, Stress2);
				this->AverageVector(AverageStrain, Strain1, Strain2);
			}
			else
			{
				AverageStress = this->GetValue(STRESS_VECTOR);
				AverageStrain = this->GetValue(STRAIN_VECTOR);
			}

			if (this->GetIteration() < 3) // Computes the l_char on each side only once at each time step
			{
				this->CalculateLchar(this, elem_neigb[cont], cont);
			}
			double l_char = this->Get_l_char(cont);

			this->IntegrateStressDamageMechanics(IntegratedStressVector, damagee, AverageStrain, AverageStress, cont, l_char);
			damage[cont] = damagee;
			this->Set_NonConvergeddamages(damagee, cont);

		} // Loop Over Edges

		Vector TwoMaxDamages;
		TwoMaxDamages.resize(2);
		this->Get2MaxValues(TwoMaxDamages, damage[0], damage[1], damage[2]);
		double damage_element = (TwoMaxDamages[0] + TwoMaxDamages[1]) * 0.5;
		if (damage_element >= 0.999)
		{
			damage_element = 0.999;
		}
		this->Set_NonConvergeddamage(damage_element);

		const Vector &StressVector = this->GetValue(STRESS_VECTOR);
		IntegratedStressVector = (1 - damage_element) * StressVector;

		this->SetIntegratedStressVector(IntegratedStressVector);

		Matrix ConstitutiveMatrix = ZeroMatrix(voigt_size, voigt_size);
		double E = this->GetProperties()[YOUNG_MODULUS];
		double nu = this->GetProperties()[POISSON_RATIO];
		this->CalculateConstitutiveMatrix(ConstitutiveMatrix, E, nu);

		noalias(rLeftHandSideMatrix) += prod(trans(B), IntegrationWeight * (1 - damage_element) * Matrix(prod(ConstitutiveMatrix, B))); //LHS

		Vector VolumeForce = ZeroVector(dimension);
		VolumeForce = this->CalculateVolumeForce(VolumeForce, N);
		// RHS
		for (unsigned int i = 0; i < number_of_nodes; i++)
		{
			int index = dimension * i;
			for (unsigned int j = 0; j < dimension; j++)
			{
				rRightHandSideVector[index + j] += IntegrationWeight * N[i] * VolumeForce[j];
			}
		}

		//compute and add internal forces (RHS = rRightHandSideVector = Fext - Fint)
		noalias(rRightHandSideVector) -= IntegrationWeight * prod(trans(B), (1 - damage_element) * StressVector);

		Vector NodalRHS = ZeroVector(6);
		#pragma omp critical
		{
			Geometry<Node<3>> &nodes_element = this->GetGeometry();

			// Loop Over nodes to apply the DEM contact forces to the FEM
			for (int i = 0; i < 3; i++)
			{
				bool IsDEM = nodes_element[i].GetValue(IS_DEM);
				bool NodalForceApplied = nodes_element[i].GetValue(NODAL_FORCE_APPLIED);

				if (IsDEM == true && NodalForceApplied == false)
				{
					double ForceX = nodes_element[i].GetValue(NODAL_FORCE_X);
					double ForceY = nodes_element[i].GetValue(NODAL_FORCE_Y);

					NodalRHS[2 * i] += ForceX;
					NodalRHS[2 * i + 1] += ForceY;

					nodes_element[i].SetValue(NODAL_FORCE_APPLIED, true);
				}
			}
		}
		// Add nodal contact forces from the DEM
		noalias(rRightHandSideVector) += NodalRHS;
	}
	KRATOS_CATCH("")
}

void FemDem2DElement::CalculateDeformationMatrix(Matrix &rB, const Matrix &rDN_DX)
{
	const unsigned int number_of_nodes = GetGeometry().PointsNumber();
	const unsigned int dimension = GetGeometry().WorkingSpaceDimension();
	unsigned int voigt_size = dimension * (dimension + 1) * 0.5;

	if (rB.size1() != voigt_size || rB.size2() != dimension * number_of_nodes)
		rB.resize(voigt_size, dimension * number_of_nodes, false);

	for (unsigned int i = 0; i < number_of_nodes; i++)
	{
		unsigned int index = 2 * i;

		rB(0, index + 0) = rDN_DX(i, 0);
		rB(0, index + 1) = 0.0;
		rB(1, index + 0) = 0.0;
		rB(1, index + 1) = rDN_DX(i, 1);
		rB(2, index + 0) = rDN_DX(i, 1);
		rB(2, index + 1) = rDN_DX(i, 0);
	}
}

void FemDem2DElement::CalculateConstitutiveMatrix(Matrix &rConstitutiveMatrix, const double &rYoungModulus,
												  const double &rPoissonCoefficient)
{
	rConstitutiveMatrix.clear();

	if (this->GetProperties()[THICKNESS] == 1)
	{
		// Plane strain constitutive matrix
		rConstitutiveMatrix(0, 0) = (rYoungModulus * (1.0 - rPoissonCoefficient) / ((1.0 + rPoissonCoefficient) * (1.0 - 2.0 * rPoissonCoefficient)));
		rConstitutiveMatrix(1, 1) = rConstitutiveMatrix(0, 0);
		rConstitutiveMatrix(2, 2) = rConstitutiveMatrix(0, 0) * (1.0 - 2.0 * rPoissonCoefficient) / (2.0 * (1.0 - rPoissonCoefficient));
		rConstitutiveMatrix(0, 1) = rConstitutiveMatrix(0, 0) * rPoissonCoefficient / (1.0 - rPoissonCoefficient);
		rConstitutiveMatrix(1, 0) = rConstitutiveMatrix(0, 1);
	}
	else
	{
		// Plane stress constitutive matrix
		rConstitutiveMatrix(0, 0) = (rYoungModulus) / (1.0 - rPoissonCoefficient * rPoissonCoefficient);
		rConstitutiveMatrix(1, 1) = rConstitutiveMatrix(0, 0);
		rConstitutiveMatrix(2, 2) = rConstitutiveMatrix(0, 0) * (1.0 - rPoissonCoefficient) * 0.5;
		rConstitutiveMatrix(0, 1) = rConstitutiveMatrix(0, 0) * rPoissonCoefficient;
		rConstitutiveMatrix(1, 0) = rConstitutiveMatrix(0, 1);
	}
}

void FemDem2DElement::CalculateDN_DX(Matrix &rDN_DX, int PointNumber)
{
	const unsigned int number_of_nodes = GetGeometry().size();
	const unsigned int dimension = GetGeometry().WorkingSpaceDimension();
	//reading integration points
	const GeometryType::IntegrationPointsArrayType &integration_points = GetGeometry().IntegrationPoints(mThisIntegrationMethod);

	//get the shape functions [N] (for the order of the default integration method)
	const Matrix &Ncontainer = GetGeometry().ShapeFunctionsValues(mThisIntegrationMethod);

	//get the shape functions parent coordinates derivative [dN/d�] (for the order of the default integration method)
	const GeometryType::ShapeFunctionsGradientsType &DN_De = GetGeometry().ShapeFunctionsLocalGradients(mThisIntegrationMethod);
	//calculate delta position (here coincides with the current displacement)
	Matrix DeltaPosition = ZeroMatrix(number_of_nodes, dimension);
	DeltaPosition = this->CalculateDeltaPosition(DeltaPosition);
	//KRATOS_WATCH(DeltaPosition)
	//calculating the reference jacobian from cartesian coordinates to parent coordinates for all integration points [dx_n/d�]
	GeometryType::JacobiansType J;
	J.resize(1, false);
	J[0] = ZeroMatrix(1, 1);
	J = GetGeometry().Jacobian(J, mThisIntegrationMethod, DeltaPosition);
	//a.-compute element kinematics

	//calculating the inverse of the jacobian for this integration point[d�/dx_n]
	Matrix InvJ = ZeroMatrix(dimension, dimension);
	double detJ = 0;
	MathUtils<double>::InvertMatrix(J[PointNumber], InvJ, detJ);

	if (detJ < 0)
		KRATOS_THROW_ERROR(std::invalid_argument, " SMALL DISPLACEMENT ELEMENT INVERTED: |J|<0 ) detJ = ", detJ)

	//compute cartesian derivatives for this integration point  [dN/dx_n]
	rDN_DX = prod(DN_De[PointNumber], InvJ);
}

void FemDem2DElement::CalculateInfinitesimalStrain(Vector &rStrainVector, const Matrix &rDN_DX)
{
	KRATOS_TRY

	const unsigned int number_of_nodes = GetGeometry().PointsNumber();
	const unsigned int dimension = GetGeometry().WorkingSpaceDimension();

	Matrix H = zero_matrix<double>(dimension); //[dU/dx_n]

	for (unsigned int i = 0; i < number_of_nodes; i++)
	{

		array_1d<double, 3> &Displacement = GetGeometry()[i].FastGetSolutionStepValue(DISPLACEMENT);

		H(0, 0) += Displacement[0] * rDN_DX(i, 0);
		H(0, 1) += Displacement[0] * rDN_DX(i, 1);
		H(1, 0) += Displacement[1] * rDN_DX(i, 0);
		H(1, 1) += Displacement[1] * rDN_DX(i, 1);
	}
	//Infinitesimal Strain Calculation
	if (rStrainVector.size() != 3)
		rStrainVector.resize(3, false);

	rStrainVector[0] = H(0, 0);
	rStrainVector[1] = H(1, 1);
	rStrainVector[2] = (H(0, 1) + H(1, 0)); // xy

	KRATOS_CATCH("")
}

void FemDem2DElement::CalculateStressVector(Vector &rStressVector, const Matrix &rConstitutiveMAtrix, const Vector &rInfinitesimalStrainVector)
{
	noalias(rStressVector) = prod(rConstitutiveMAtrix, rInfinitesimalStrainVector);
}

void FemDem2DElement::CalculatePrincipalStress(Vector &rPrincipalStressVector, const Vector StressVector)
{
	rPrincipalStressVector.resize(2);
	rPrincipalStressVector[0] = 0.5 * (StressVector[0] + StressVector[1]) + std::sqrt(std::pow(0.5 * (StressVector[0] - StressVector[1]), 2) + std::pow(StressVector[2], 2));
	rPrincipalStressVector[1] = 0.5 * (StressVector[0] + StressVector[1]) - std::sqrt(std::pow(0.5 * (StressVector[0] - StressVector[1]), 2) + std::pow(StressVector[2], 2));
}

void FemDem2DElement::FinalizeNonLinearIteration(ProcessInfo &CurrentProcessInfo)
{
}

void FemDem2DElement::AverageVector(Vector &rAverageVector, const Vector &v, const Vector &w)
{
	int n = v.size();
	int m = w.size();
	if (n != m)
		KRATOS_ERROR << "The dimension of the vectors are different or null";
	rAverageVector.resize(n);
	for (int cont = 0; cont < n; cont++)
	{
		rAverageVector[cont] = (v[cont] + w[cont]) * 0.5;
	}
}

void FemDem2DElement::GetValueOnIntegrationPoints(
	const Variable<double> &rVariable,
	std::vector<double> &rValues,
	const ProcessInfo &rCurrentProcessInfo)
{
	if (rVariable == DAMAGE_ELEMENT || rVariable == IS_DAMAGED || rVariable == STRESS_THRESHOLD)
	{
		CalculateOnIntegrationPoints(rVariable, rValues, rCurrentProcessInfo);
	}
}

void FemDem2DElement::GetValueOnIntegrationPoints(
	const Variable<Vector> &rVariable,
	std::vector<Vector> &rValues,
	const ProcessInfo &rCurrentProcessInfo)
{

	if (rVariable == STRAIN_VECTOR)
	{
		CalculateOnIntegrationPoints(rVariable, rValues, rCurrentProcessInfo);
	}
	else if (rVariable == STRESS_VECTOR)
	{
		CalculateOnIntegrationPoints(rVariable, rValues, rCurrentProcessInfo);
	}
	else if (rVariable == STRESS_VECTOR_INTEGRATED)
	{
		CalculateOnIntegrationPoints(rVariable, rValues, rCurrentProcessInfo);
	}

	const unsigned int &integration_points_number = mConstitutiveLawVector.size();

	if (rValues.size() != integration_points_number)
		rValues.resize(integration_points_number);

	if (rVariable == PK2_STRESS_TENSOR || rVariable == CAUCHY_STRESS_TENSOR)
	{
		CalculateOnIntegrationPoints(rVariable, rValues, rCurrentProcessInfo);
	}
	else if (rVariable == PK2_STRESS_VECTOR || rVariable == CAUCHY_STRESS_VECTOR)
	{
		CalculateOnIntegrationPoints(rVariable, rValues, rCurrentProcessInfo);
	}
	else if (rVariable == GREEN_LAGRANGE_STRAIN_TENSOR || rVariable == ALMANSI_STRAIN_TENSOR)
	{
		CalculateOnIntegrationPoints(rVariable, rValues, rCurrentProcessInfo);
	}
	else
	{
		for (unsigned int PointNumber = 0; PointNumber < integration_points_number; PointNumber++)
		{
			rValues[PointNumber] = mConstitutiveLawVector[PointNumber]->GetValue(rVariable, rValues[PointNumber]);
		}
	}
}

// DOUBLE VARIABLES
void FemDem2DElement::CalculateOnIntegrationPoints(
	const Variable<double> &rVariable,
	std::vector<double> &rOutput,
	const ProcessInfo &rCurrentProcessInfo)
{
	if (rVariable == DAMAGE_ELEMENT)
	{
		rOutput.resize(1);
		for (unsigned int PointNumber = 0; PointNumber < 1; PointNumber++)
		{
			rOutput[PointNumber] = double(this->GetValue(DAMAGE_ELEMENT));
		}
	}
	else if (rVariable == IS_DAMAGED)
	{
		rOutput.resize(1);
		for (unsigned int PointNumber = 0; PointNumber < 1; PointNumber++)
		{
			rOutput[PointNumber] = double(this->GetValue(IS_DAMAGED));
		}
	}
	else if (rVariable == STRESS_THRESHOLD)
	{
		rOutput.resize(1);
		for (unsigned int PointNumber = 0; PointNumber < 1; PointNumber++)
		{
			rOutput[PointNumber] = double(this->GetValue(STRESS_THRESHOLD));
		}
	}
}

// VECTOR VARIABLES
void FemDem2DElement::CalculateOnIntegrationPoints(
	const Variable<Vector> &rVariable,
	std::vector<Vector> &rOutput,
	const ProcessInfo &rCurrentProcessInfo)
{
	KRATOS_TRY

	if (rVariable == STRESS_VECTOR)
	{
		rOutput[0] = this->GetValue(STRESS_VECTOR);
	}
	else if (rVariable == STRAIN_VECTOR)
	{
		rOutput[0] = this->GetValue(STRAIN_VECTOR);
	}
	else if (rVariable == STRESS_VECTOR_INTEGRATED)
	{
		rOutput[0] = this->GetIntegratedStressVector();
	}

	const unsigned int &integration_points_number = GetGeometry().IntegrationPointsNumber(mThisIntegrationMethod);

	if (rOutput.size() != integration_points_number)
		rOutput.resize(integration_points_number);
	else if (rVariable == GREEN_LAGRANGE_STRAIN_VECTOR || rVariable == ALMANSI_STRAIN_VECTOR)
	{
		//create and initialize element variables:
		ElementVariables Variables;
		this->InitializeElementVariables(Variables, rCurrentProcessInfo);

		//reading integration points
		for (unsigned int PointNumber = 0; PointNumber < mConstitutiveLawVector.size(); PointNumber++)
		{
			//compute element kinematics B, F, DN_DX ...
			this->CalculateKinematics(Variables, PointNumber);

			if (rOutput[PointNumber].size() != Variables.StrainVector.size())
				rOutput[PointNumber].resize(Variables.StrainVector.size(), false);

			rOutput[PointNumber] = Variables.StrainVector;
		}
	}
	else
	{
		for (unsigned int ii = 0; ii < mConstitutiveLawVector.size(); ii++)
		{
			rOutput[ii] = mConstitutiveLawVector[ii]->GetValue(rVariable, rOutput[ii]);
		}
	}

	KRATOS_CATCH("")
}

double FemDem2DElement::CalculateLchar(FemDem2DElement *CurrentElement, const Element &NeibElement, int cont)
{
	Geometry<Node<3>> &NodesElem1 = CurrentElement->GetGeometry(); // 3 nodes of the Element 1
	Geometry<Node<3>> NodesElem2 = NeibElement.GetGeometry();	  // "         " 2
	Vector Xcoord, Ycoord;
	Xcoord.resize(3);
	Ycoord.resize(3);

	// Let's find the two shared nodes between the 2 elements
	int aux = 0;
	double l_char = 0;
	for (int cont = 0; cont < 3; cont++)
	{
		for (int cont2 = 0; cont2 < 3; cont2++)
		{
			if (NodesElem1[cont].Id() == NodesElem2[cont2].Id())
			{
				Xcoord[aux] = NodesElem1[cont].X0();
				Ycoord[aux] = NodesElem1[cont].Y0();
				aux++; // aux > 3 if the two elements are the same one (in fact aux == 9)
			}
		}
	} // End finding nodes

	// Computation of the l_char
	if (aux < 3)
	{																									   // It is not an edge element --> The 2 elements are not equal
		l_char = std::pow((std::pow(Xcoord[0] - Xcoord[1], 2) + std::pow(Ycoord[0] - Ycoord[1], 2)), 0.5); // Length of the edge between 2 elements                                                                  // Currently the characteristic length is the edge length (can be modified)
	}
	else
	{ // Edge element
		double ElementArea = std::abs(this->GetGeometry().Area());
		l_char = std::sqrt(4.0 * ElementArea / std::sqrt(3)); // Cervera's Formula
	}											  // l_char computed

	CurrentElement->SetCharacteristicLength(l_char, cont); // Storages the l_char of this side
	CurrentElement->IterationPlus();

	return 0.0;
}

void FemDem2DElement::Get2MaxValues(Vector &MaxValues, double a, double b, double c)
{
	MaxValues.resize(2);
	Vector V;
	V.resize(3);
	V[0] = a;
	V[1] = b;
	V[2] = c;
	int n = 3, imin = 0;

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n - 1; j++)
		{
			if (V[j] > V[j + 1])
			{
				double aux = V[j];
				V[j] = V[j + 1];
				V[j + 1] = aux;
			}
		}
	}
	MaxValues[0] = V[2];
	MaxValues[1] = V[1];
}

void FemDem2DElement::Get2MinValues(Vector &MaxValues, double a, double b, double c)
{
	MaxValues.resize(2);
	Vector V;
	V.resize(3);
	V[0] = a;
	V[1] = b;
	V[2] = c;
	int n = 3, imin = 0;

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n - 1; j++)
		{
			if (V[j] > V[j + 1])
			{
				double aux = V[j];
				V[j] = V[j + 1];
				V[j + 1] = aux;
			}
		}
	}
	MaxValues[0] = V[1];
	MaxValues[1] = V[0];
}
double FemDem2DElement::CalculateI1Invariant(double sigma1, double sigma2) { return sigma1 + sigma2; }
double FemDem2DElement::CalculateJ2Invariant(double sigma1, double sigma2)
{
	return (std::pow((sigma1 - sigma2), 2) + std::pow(sigma1, 2) + std::pow(sigma2, 2)) / 6.0;
}
double FemDem2DElement::CalculateJ3Invariant(double sigma1, double sigma2, double I1)
{
	return (sigma1 - I1 / 3.0) * ((sigma2 - I1 / 3.0)) * (-I1 / 3.0);
}
double FemDem2DElement::CalculateLodeAngle(double J2, double J3)
{
	double sint3;
	sint3 = (-3.0 * std::sqrt(3.0) * J3) / (2.0 * J2 * std::sqrt(J2));
	if (sint3 < -0.95)
	{
		sint3 = -1;
	}
	if (sint3 > 0.95)
	{
		sint3 = 1;
	}
	return std::asin(sint3) / 3.0;
}

void FemDem2DElement::CalculateMassMatrix(MatrixType &rMassMatrix, ProcessInfo &rCurrentProcessInfo)
{
	KRATOS_TRY

	bool ComputeLumpedMassMatrix = false;
	if (rCurrentProcessInfo.Has(COMPUTE_LUMPED_MASS_MATRIX))
		if (rCurrentProcessInfo[COMPUTE_LUMPED_MASS_MATRIX] == true)
			ComputeLumpedMassMatrix = true;

	if (ComputeLumpedMassMatrix == false)
	{

		//create local system components
		LocalSystemComponents LocalSystem;

		//calculation flags
		LocalSystem.CalculationFlags.Set(SmallDisplacementElement::COMPUTE_LHS_MATRIX);

		VectorType RightHandSideVector = Vector();

		//Initialize sizes for the system components:
		this->InitializeSystemMatrices(rMassMatrix, RightHandSideVector, LocalSystem.CalculationFlags);

		//Set Variables to Local system components
		LocalSystem.SetLeftHandSideMatrix(rMassMatrix);
		LocalSystem.SetRightHandSideVector(RightHandSideVector);

		//Calculate elemental system
		CalculateDynamicSystem(LocalSystem, rCurrentProcessInfo);
	}
	else
	{

		//lumped
		unsigned int dimension = GetGeometry().WorkingSpaceDimension();
		const unsigned int number_of_nodes = GetGeometry().PointsNumber();
		unsigned int MatSize = dimension * number_of_nodes;

		if (rMassMatrix.size1() != MatSize)
			rMassMatrix.resize(MatSize, MatSize, false);

		noalias(rMassMatrix) = ZeroMatrix(MatSize, MatSize);

		double TotalMass = 0;
		TotalMass = this->CalculateTotalMass(TotalMass, rCurrentProcessInfo);

		Vector LumpFact(number_of_nodes);
		noalias(LumpFact) = ZeroVector(number_of_nodes);

		LumpFact = GetGeometry().LumpingFactors(LumpFact);

		for (unsigned int i = 0; i < number_of_nodes; i++)
		{
			double temp = LumpFact[i] * TotalMass;

			for (unsigned int j = 0; j < dimension; j++)
			{
				unsigned int index = i * dimension + j;
				rMassMatrix(index, index) = temp;
			}
		}
	}

	KRATOS_CATCH("")
}
Vector &FemDem2DElement::CalculateVolumeForce(Vector &rVolumeForce, const Vector &rN)
{
	KRATOS_TRY

	const unsigned int number_of_nodes = GetGeometry().PointsNumber();
	const unsigned int dimension = GetGeometry().WorkingSpaceDimension();

	if (rVolumeForce.size() != dimension)
		rVolumeForce.resize(dimension, false);

	noalias(rVolumeForce) = ZeroVector(dimension);

	for (unsigned int j = 0; j < number_of_nodes; j++)
	{
		if (GetGeometry()[j].SolutionStepsDataHas(VOLUME_ACCELERATION))
		{ // it must be checked once at the begining only
			array_1d<double, 3> &VolumeAcceleration = GetGeometry()[j].FastGetSolutionStepValue(VOLUME_ACCELERATION);
			for (unsigned int i = 0; i < dimension; i++)
				rVolumeForce[i] += rN[j] * VolumeAcceleration[i];
		}
	}

	rVolumeForce *= GetProperties()[DENSITY];
	return rVolumeForce;

	KRATOS_CATCH("")
}

double FemDem2DElement::GetMaxValue(Vector Strain)
{
	Vector V;
	int n = Strain.size();
	V.resize(n);

	for (int cont = 0; cont < n; cont++)
	{
		V[cont] = Strain[cont];
	}

	int imin = 0;

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n - 1; j++)
		{
			if (V[j] > V[j + 1])
			{
				double aux = V[j];
				V[j] = V[j + 1];
				V[j + 1] = aux;
			}
		}
	}

	return V[n - 1];
}

double FemDem2DElement::GetMaxAbsValue(Vector Strain)
{
	Vector V;
	int n = Strain.size();
	V.resize(n);

	for (int cont = 0; cont < n; cont++)
	{
		V[cont] = std::abs(Strain[cont]);
	}

	int imin = 0;

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n - 1; j++)
		{
			if (V[j] > V[j + 1])
			{
				double aux = V[j];
				V[j] = V[j + 1];
				V[j + 1] = aux;
			}
		}
	}

	return V[n - 1];
}

double FemDem2DElement::GetMinAbsValue(Vector Strain)
{
	Vector V;
	V.resize(3);
	V[1] = std::abs(Strain[1]);
	V[0] = std::abs(Strain[0]);
	V[2] = std::abs(Strain[2]);
	int n = 3, imin = 0;

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n - 1; j++)
		{
			if (V[j] > V[j + 1])
			{
				double aux = V[j];
				V[j] = V[j + 1];
				V[j + 1] = aux;
			}
		}
	}

	return V[0];
}

// ****** Tangent Constitutive Tensor by Numerical Derivation ******
void FemDem2DElement::PerturbateStrainComponent(
	const Vector &rStrainVector,
	Vector &PertubatedStrain,
	const double perturbation,
	int component)
{
	PertubatedStrain = rStrainVector;
	PertubatedStrain[component] += perturbation;
}

double FemDem2DElement::CalculatePerturbation(const Vector &StrainVector, int component)
{
	double Pert = 0.0;
	if (StrainVector[component] != 0.0)
		Pert = (1e-5) * StrainVector[component];
	else
		Pert = (1e-5) * GetMinAbsValue(StrainVector);
	if (Pert < this->GetMaxAbsValue(StrainVector) * (1e-10))
	{
		Pert = GetMaxAbsValue(StrainVector) * (1e-10);
	}

	return Pert;
}

void FemDem2DElement::CalculateTangentTensor(
	Matrix &rTangentTensor,
	const Vector &StrainVector,
	const Vector &IntegratedStressVector,
	int cont,
	double l_char)
{
	rTangentTensor.resize(3, 3);

	double E = this->GetProperties()[YOUNG_MODULUS];
	double nu = this->GetProperties()[POISSON_RATIO];
	Matrix ConstitutiveMatrix = ZeroMatrix(3, 3);
	this->CalculateConstitutiveMatrix(ConstitutiveMatrix, E, nu);

	// Perturbed Strain Vectors
	Vector PerturbedStrain1 = ZeroVector(3);
	Vector PerturbedStrain2 = ZeroVector(3);
	Vector PerturbedStrain3 = ZeroVector(3);

	Vector PerturbedIntegratedStress1 = ZeroVector(3);
	Vector PerturbedIntegratedStress2 = ZeroVector(3);
	Vector PerturbedIntegratedStress3 = ZeroVector(3);

	Vector PerturbedStress1 = ZeroVector(3);
	Vector PerturbedStress2 = ZeroVector(3);
	Vector PerturbedStress3 = ZeroVector(3);

	Vector DeltaStress1 = ZeroVector(3);
	Vector DeltaStress2 = ZeroVector(3);
	Vector DeltaStress3 = ZeroVector(3);

	// Calculation of the perturbations
	double Perturbation1 = this->CalculatePerturbation(StrainVector, 0);
	double Perturbation2 = this->CalculatePerturbation(StrainVector, 1);
	double Perturbation3 = this->CalculatePerturbation(StrainVector, 2);

	// Calculation of the Perturbed Strain Vectors
	this->PerturbateStrainComponent(StrainVector, PerturbedStrain1, Perturbation1, 0);
	this->PerturbateStrainComponent(StrainVector, PerturbedStrain2, Perturbation2, 1);
	this->PerturbateStrainComponent(StrainVector, PerturbedStrain3, Perturbation3, 2);

	// Calculation of the Perturbed Predictive Stress Vectors
	this->CalculateStressVector(PerturbedStress1, ConstitutiveMatrix, PerturbedStrain1);
	this->CalculateStressVector(PerturbedStress2, ConstitutiveMatrix, PerturbedStrain2);
	this->CalculateStressVector(PerturbedStress3, ConstitutiveMatrix, PerturbedStrain3);

	// Integration of the Perturbed Predictive Stress Vectors
	double damage1 = 0.0, damage2 = 0.0, damage3 = 0.0;

	this->TangentModifiedMohrCoulombCriterion(PerturbedIntegratedStress1, damage1, PerturbedStress1, cont, l_char);
	this->TangentModifiedMohrCoulombCriterion(PerturbedIntegratedStress2, damage2, PerturbedStress2, cont, l_char);
	this->TangentModifiedMohrCoulombCriterion(PerturbedIntegratedStress3, damage3, PerturbedStress3, cont, l_char);

	DeltaStress1 = PerturbedIntegratedStress1 - IntegratedStressVector;
	DeltaStress2 = PerturbedIntegratedStress2 - IntegratedStressVector;
	DeltaStress3 = PerturbedIntegratedStress3 - IntegratedStressVector;

	for (int row = 0; row < 3; row++) // DeltaStress is the i column of the Tangent Tensor
	{
		rTangentTensor(row, 0) = DeltaStress1[row] / (Perturbation1);
		rTangentTensor(row, 1) = DeltaStress2[row] / (Perturbation2);
		rTangentTensor(row, 2) = DeltaStress3[row] / (Perturbation3);
	}
}

void FemDem2DElement::TangentModifiedMohrCoulombCriterion(
	Vector &rIntegratedStress,
	double &damage,
	const Vector &StressVector,
	int cont,
	double l_char)
{
	rIntegratedStress.resize(3);
	Vector PrincipalStressVector = ZeroVector(2);
	this->CalculatePrincipalStress(PrincipalStressVector, StressVector);

	double sigma_c = 0.0, sigma_t = 0.0, friction_angle = 0.0, E = 0.0, Gt = 0.0;
	sigma_c = this->GetProperties()[YIELD_STRESS_C];
	sigma_t = this->GetProperties()[YIELD_STRESS_T];
	friction_angle = this->GetProperties()[INTERNAL_FRICTION_ANGLE] * 3.14159 / 180; // In radians!
	E = this->GetProperties()[YOUNG_MODULUS];
	Gt = this->GetProperties()[FRAC_ENERGY_T];

	// Check input variables
	if (friction_angle < 1e-24)
	{
		friction_angle = 32 * 3.14159 / 180;
		std::cout << "Friction Angle not defined, assumed equal to 32� " << std::endl;
	}
	if (sigma_c < 1e-24)
	{
		KRATOS_ERROR << " ERROR: Yield stress in compression not defined, include YIELD_STRESS_C in .mdpa ";
	}
	if (sigma_t < 1e-24)
	{
		KRATOS_ERROR << " ERROR: Yield stress in tension not defined, include YIELD_STRESS_T in .mdpa ";
	}
	if (Gt < 1e-24)
	{
		KRATOS_ERROR << " ERROR: Fracture Energy not defined in the model part, include FRAC_ENERGY_T in .mdpa ";
	}

	double K1, K2, K3, Rmorh, R, alpha_r, c_max, theta, c_threshold;
	R = std::abs(sigma_c / sigma_t);
	Rmorh = std::pow(tan((3.14159 / 4) + friction_angle / 2), 2);
	alpha_r = R / Rmorh;
	c_max = std::abs(sigma_c);

	double I1, J2, J3;
	I1 = CalculateI1Invariant(PrincipalStressVector[0], PrincipalStressVector[1]);
	J2 = CalculateJ2Invariant(PrincipalStressVector[0], PrincipalStressVector[1]);
	J3 = CalculateJ3Invariant(PrincipalStressVector[0], PrincipalStressVector[1], I1);
	K1 = 0.5 * (1 + alpha_r) - 0.5 * (1 - alpha_r) * std::sin(friction_angle);
	K2 = 0.5 * (1 + alpha_r) - 0.5 * (1 - alpha_r) / std::sin(friction_angle);
	K3 = 0.5 * (1 + alpha_r) * std::sin(friction_angle) - 0.5 * (1 - alpha_r);

	double n = sigma_c / sigma_t;

	double A = 1.00 / (n * n * Gt * E / (l_char * std::pow(sigma_c, 2)) - 0.5);
	if (A < 0)
	{
		KRATOS_THROW_ERROR(std::invalid_argument, " 'A' damage parameter lower than zero --> Increase FRAC_ENERGY_T", A)
	}

	double f = 0.0, F = 0.0; /// F = f-c = 0 classical definition of yield surface

	// Check Modified Mohr-Coulomb criterion
	if (PrincipalStressVector[0] == 0 && PrincipalStressVector[1] == 0)
	{
		f = 0;
	}
	else
	{
		theta = CalculateLodeAngle(J2, J3);
		f = (2.00 * std::tan(3.14159 * 0.25 + friction_angle * 0.5) / std::cos(friction_angle)) * ((I1 * K3 / 3) + std::sqrt(J2) * (K1 * std::cos(theta) - K2 * std::sin(theta) * std::sin(friction_angle) / std::sqrt(3.0)));
	}

	if (this->GetThreshold(cont) == 0)
	{
		this->SetThreshold(c_max, cont);
		this->SetValue(INITIAL_THRESHOLD, c_max);
	} // 1st iteration sets threshold as c_max

	c_threshold = this->GetThreshold(cont);

	F = f - c_threshold;

	if (F <= 0) // Elastic region --> Damage is constant
	{
		damage = this->GetConvergedDamages(cont);
	}
	else
	{
		damage = 1.0 - (c_max / f) * std::exp(A * (1 - f / c_max)); // Exponential softening law
		if (damage > 0.99)
		{
			damage = 0.99;
		}
	}

	rIntegratedStress = StressVector;
	rIntegratedStress *= (1.0 - damage);
}

// ******* DAMAGE MECHANICS YIELD SURFACES AND EXPONENTIAL SOFTENING ********
void FemDem2DElement::IntegrateStressDamageMechanics(
	Vector &rIntegratedStress,
	double &damage,
	const Vector StrainVector,
	const Vector StressVector,
	int cont,
	double l_char)
{
	const std::string yield_surface = this->GetProperties()[YIELD_SURFACE];

	if (yield_surface == "ModifiedMohrCoulomb")
	{
		this->ModifiedMohrCoulombCriterion(rIntegratedStress, damage, StressVector, cont, l_char);
	}
	else if (yield_surface == "SimoJu")
	{
		this->SimoJuCriterion(rIntegratedStress, damage, StrainVector, StressVector, cont, l_char);
	}
	else if (yield_surface == "Rankine")
	{
		this->RankineCriterion(rIntegratedStress, damage, StressVector, cont, l_char);
	}
	else if (yield_surface == "DruckerPrager")
	{
		this->DruckerPragerCriterion(rIntegratedStress, damage, StressVector, cont, l_char);
	}
	else if (yield_surface == "RankineFragile")
	{
		this->RankineFragileLaw(rIntegratedStress, damage, StressVector, cont, l_char);
	}
	else
	{
		KRATOS_ERROR << " Yield Surface not defined ";
	}
}

void FemDem2DElement::ModifiedMohrCoulombCriterion(Vector &rIntegratedStress, double &damage, const Vector &StressVector, int cont, double l_char)
{
	rIntegratedStress.resize(3);
	Vector PrincipalStressVector = ZeroVector(2);
	this->CalculatePrincipalStress(PrincipalStressVector, StressVector);

	double sigma_c = 0.0, sigma_t = 0.0, friction_angle = 0.0, E = 0.0, Gt = 0.0;
	sigma_c = this->GetProperties()[YIELD_STRESS_C];
	sigma_t = this->GetProperties()[YIELD_STRESS_T];
	friction_angle = this->GetProperties()[INTERNAL_FRICTION_ANGLE] * 3.14159 / 180; // In radians!
	E = this->GetProperties()[YOUNG_MODULUS];
	Gt = this->GetProperties()[FRAC_ENERGY_T];

	// Check input variables
	if (friction_angle < 1e-24)
	{
		friction_angle = 32 * 3.14159 / 180;
		std::cout << "Friction Angle not defined, assumed equal to 32 " << std::endl;
	}
	if (sigma_c < 1e-24)
	{
		KRATOS_ERROR << " ERROR: Yield stress in compression not defined, include YIELD_STRESS_C in .mdpa ";
	}
	if (sigma_t < 1e-24)
	{
		KRATOS_ERROR << " ERROR: Yield stress in tension not defined, include YIELD_STRESS_T in .mdpa ";
	}
	if (Gt < 1e-24)
	{
		KRATOS_ERROR << " ERROR: Fracture Energy not defined in the model part, include FRAC_ENERGY_T in .mdpa ";
	}

	double K1, K2, K3, Rmorh, R, alpha_r, c_max, theta, c_threshold;
	R = std::abs(sigma_c / sigma_t);
	Rmorh = std::pow(tan((3.14159 / 4) + friction_angle / 2), 2);
	alpha_r = R / Rmorh;
	c_max = std::abs(sigma_c);

	double I1, J2, J3;
	I1 = CalculateI1Invariant(PrincipalStressVector[0], PrincipalStressVector[1]);
	J2 = CalculateJ2Invariant(PrincipalStressVector[0], PrincipalStressVector[1]);
	J3 = CalculateJ3Invariant(PrincipalStressVector[0], PrincipalStressVector[1], I1);
	K1 = 0.5 * (1 + alpha_r) - 0.5 * (1 - alpha_r) * std::sin(friction_angle);
	K2 = 0.5 * (1 + alpha_r) - 0.5 * (1 - alpha_r) / std::sin(friction_angle);
	K3 = 0.5 * (1 + alpha_r) * std::sin(friction_angle) - 0.5 * (1 - alpha_r);

	double n = sigma_c / sigma_t;
	double ElementArea = this->GetGeometry().Area();

	double A = 1.00 / (n * n * Gt * E / (l_char * std::pow(sigma_c, 2)) - 0.5);
	if (A < 0)
	{
		KRATOS_THROW_ERROR(std::invalid_argument, " 'A' damage parameter lower than zero --> Increase FRAC_ENERGY_T", A)
	}

	double f = 0.0, F = 0.0; /// F = f-c = 0 classical definition of yield surface

	// Check Modified Mohr-Coulomb criterion
	if (PrincipalStressVector[0] == 0 && PrincipalStressVector[1] == 0)
	{
		f = 0;
	}
	else
	{
		theta = CalculateLodeAngle(J2, J3);
		f = (2.00 * std::tan(3.14159 * 0.25 + friction_angle * 0.5) / std::cos(friction_angle)) * ((I1 * K3 / 3) + 
			std::sqrt(J2) * (K1 * std::cos(theta) - K2 * std::sin(theta) * std::sin(friction_angle) / std::sqrt(3)));
	}

	if (this->GetThreshold(cont) == 0)
	{
		this->SetThreshold(c_max, cont);
	} // 1st iteration sets threshold as c_max
	c_threshold = this->GetThreshold(cont);
	this->Set_NonConvergedf_sigma(f, cont);

	F = f - c_threshold;

	if (F <= 0) // Elastic region --> Damage is constant
	{
		damage = this->GetConvergedDamages(cont);
	}
	else
	{
		damage = 1 - (c_max / f) * std::exp(A * (1 - f / c_max)); // Exponential softening law
		if (damage > 0.99)
		{
			damage = 0.99;
		}
	}

	rIntegratedStress = StressVector;
	rIntegratedStress *= (1 - damage);
}

void FemDem2DElement::RankineCriterion(
	Vector &rIntegratedStress,
	double &damage,
	const Vector &StressVector,
	int cont,
	double l_char)
{
	Vector PrincipalStressVector = ZeroVector(3);
	this->CalculatePrincipalStress(PrincipalStressVector, StressVector);

	double sigma_c = 0.0, sigma_t = 0.0, friction_angle = 0.0, E = 0.0,
		   Gt = 0.0, c_max = 0.0, c_threshold = 0.0;

	sigma_c = this->GetProperties()[YIELD_STRESS_C];
	sigma_t = this->GetProperties()[YIELD_STRESS_T];
	friction_angle = this->GetProperties()[INTERNAL_FRICTION_ANGLE] * 3.14159 / 180; // In radians!
	E = this->GetProperties()[YOUNG_MODULUS];
	Gt = this->GetProperties()[FRAC_ENERGY_T];
	c_max = std::abs(sigma_t);

	double ElementArea = this->GetGeometry().Area();
	//double l_char = sqrt(4 * ElementArea / sqrt(3));
	double A = 1.00 / (Gt * E / (l_char * std::pow(sigma_c, 2)) - 0.5);
	if (A < 0)
	{
		KRATOS_THROW_ERROR(std::invalid_argument, " 'A' damage parameter lower than zero --> Increase FRAC_ENERGY_T", A)
	}

	double f, F; /// F = f-c = 0 classical definition of yield surface
	f = GetMaxValue(PrincipalStressVector);

	if (this->GetThreshold(cont) == 0)
	{
		this->SetThreshold(c_max, cont);
	} // 1st iteration sets threshold as c_max
	c_threshold = this->GetThreshold(cont);
	this->Set_NonConvergedf_sigma(f, cont);

	F = f - c_threshold;

	if (F <= 0) // Elastic region --> Damage is constant
	{
		damage = this->Get_Convergeddamage();
		//this->Set_NonConvergeddamage(damage);
	}
	else
	{
		damage = 1 - (c_max / f) * std::exp(A * (1 - f / c_max)); // Exponential softening law
		if (damage > 0.99)
		{
			damage = 0.99;
		}
		//this->Set_NonConvergeddamage(damage);
	}
	rIntegratedStress = StressVector;
	rIntegratedStress *= (1 - damage);
}

void FemDem2DElement::DruckerPragerCriterion(
	Vector &rIntegratedStress,
	double &damage,
	const Vector &StressVector,
	int cont,
	double l_char)
{
	Vector PrincipalStressVector = ZeroVector(3);
	this->CalculatePrincipalStress(PrincipalStressVector, StressVector);

	double sigma_c = 0.0, sigma_t = 0.0, friction_angle = 0.0, E = 0.0, Gt = 0.0;
	sigma_c = this->GetProperties()[YIELD_STRESS_C];
	sigma_t = this->GetProperties()[YIELD_STRESS_T];
	friction_angle = this->GetProperties()[INTERNAL_FRICTION_ANGLE] * 3.14159 / 180; // In radians!
	E = this->GetProperties()[YOUNG_MODULUS];
	Gt = this->GetProperties()[FRAC_ENERGY_T];

	// Check input variables
	if (friction_angle < 1e-24)
	{
		friction_angle = 32 * 3.14159 / 180;
		std::cout << "Friction Angle not defined, assumed equal to 32deg " << std::endl;
	}
	if (sigma_c < 1e-24)
	{
		KRATOS_ERROR << " ERROR: Yield stress in compression not defined, include YIELD_STRESS_C in .mdpa ";
	}
	if (sigma_t < 1e-24)
	{
		KRATOS_ERROR << " ERROR: Yield stress in tension not defined, include YIELD_STRESS_T in .mdpa ";
	}
	if (Gt < 1e-24)
	{
		KRATOS_ERROR << " ERROR: Fracture Energy not defined in the model part, include FRAC_ENERGY_T in .mdpa ";
	}

	double c_max, c_threshold;
	c_max = std::abs(sigma_t * (3.0 + std::sin(friction_angle)) / (3.0 * std::sin(friction_angle) - 3.0));

	double I1, J2;
	I1 = CalculateI1Invariant(PrincipalStressVector[0], PrincipalStressVector[1]);
	J2 = CalculateJ2Invariant(PrincipalStressVector[0], PrincipalStressVector[1]);

	double ElementArea = this->GetGeometry().Area();
	double A = 1.00 / (Gt * E / (l_char * std::pow(sigma_c, 2)) - 0.5);
	if (A < 0)
	{
		KRATOS_THROW_ERROR(std::invalid_argument, " 'A' damage parameter lower than zero --> Increase FRAC_ENERGY_T", A)
	}

	double f, F; /// F = f-c = 0 classical definition of yield surface

	double CFL = 0.0, TEN0 = 0.0;

	// Check DruckerPrager criterion
	if (PrincipalStressVector[0] == 0 && PrincipalStressVector[1] == 0)
	{
		f = 0;
	}
	else
	{
		CFL = -std::sqrt(3.0) * (3.0 - std::sin(friction_angle)) / (3.0 * std::sin(friction_angle) - 3.0);
		TEN0 = 2.0 * I1 * std::sin(friction_angle) / (std::sqrt(3.0) * (3.0 - std::sin(friction_angle))) + std::sqrt(J2);
		f = std::abs(CFL * TEN0);
	}

	if (this->GetThreshold(cont) == 0)
	{
		this->SetThreshold(c_max, cont);
	} // 1st iteration sets threshold as c_max
	c_threshold = this->GetThreshold(cont);
	this->Set_NonConvergedf_sigma(f, cont);
	F = f - c_threshold;

	if (F <= 0) // Elastic region --> Damage is constant
	{
		damage = this->Get_Convergeddamage();
	}
	else
	{
		damage = 1.0 - (c_max / f) * std::exp(A * (1.0 - f / c_max)); // Exponential softening law
		if (damage > 0.99)
		{
			damage = 0.99;
		}
	}
	rIntegratedStress = StressVector;
	rIntegratedStress *= (1 - damage);
}

void FemDem2DElement::SimoJuCriterion(
	Vector &rIntegratedStress,
	double &damage,
	const Vector &StrainVector,
	const Vector &StressVector,
	int cont,
	double l_char)
{
	Vector PrincipalStressVector = ZeroVector(3);
	this->CalculatePrincipalStress(PrincipalStressVector, StressVector);

	double sigma_t = 0.0, E = 0.0, Gt = 0.0, sigma_c = 0.0, n = 0;
	sigma_t = this->GetProperties()[YIELD_STRESS_T];
	sigma_c = this->GetProperties()[YIELD_STRESS_C];
	E = this->GetProperties()[YOUNG_MODULUS];
	Gt = this->GetProperties()[FRAC_ENERGY_T];

	double c_max, c_threshold;
	n = std::abs(sigma_c / sigma_t);
	c_max = std::abs(sigma_c) / std::sqrt(E);

	double SumA = 0.0, SumB = 0.0, SumC = 0.0, ere0 = 0.0, ere1 = 0.0;
	for (int cont = 0; cont < 1; cont++)
	{
		SumA += std::abs(PrincipalStressVector[cont]);
		SumB += 0.5 * (PrincipalStressVector[cont] + std::abs(PrincipalStressVector[cont]));
		SumC += 0.5 * (-PrincipalStressVector[cont] + std::abs(PrincipalStressVector[cont]));
	}
	ere0 = SumB / SumA;
	ere1 = SumC / SumA;

	double f = 0, F = 0; /// F = f-c = 0 classical definition of yield surface

	// Check SimoJu criterion
	if (StrainVector[0] == 0 && StrainVector[1] == 0)
	{
		f = 0;
	}
	else
	{
		double auxf = 0.0;
		for (int cont = 0; cont < 3; cont++)
		{
			auxf += StrainVector[cont] * StressVector[cont]; // E*S
		}
		f = std::sqrt(auxf);
		f *= (ere0 * n + ere1);
	}

	if (this->GetThreshold(cont) == 0)
	{
		this->SetThreshold(c_max, cont);
	} // 1st iteration sets threshold as c_max
	c_threshold = this->GetThreshold(cont);
	this->Set_NonConvergedf_sigma(f, cont);
	F = f - c_threshold;

	double ElementArea = this->GetGeometry().Area();
	double A = 1.00 / (Gt * n * n * E / (l_char * std::pow(sigma_c, 2)) - 0.5);
	if (A < 0)
	{
		KRATOS_THROW_ERROR(std::invalid_argument, " 'A' damage parameter lower than zero --> Increase FRAC_ENERGY_T", A)
	}

	if (F <= 0) // Elastic region --> Damage is constant
	{
		damage = this->Get_Convergeddamage();
	}
	else
	{
		damage = 1.0 - (c_max / f) * std::exp(A * (1.0 - f / c_max)); // Exponential softening law
		if (damage > 0.99)
		{
			damage = 0.99;
		}
	}
	rIntegratedStress = StressVector;
	rIntegratedStress *= (1.0 - damage);
}

void FemDem2DElement::RankineFragileLaw(
	Vector &rIntegratedStress,
	double &damage,
	const Vector &StressVector,
	int cont,
	double l_char)
{
	Vector PrincipalStressVector = ZeroVector(3);
	this->CalculatePrincipalStress(PrincipalStressVector, StressVector);

	double sigma_c = 0.0, sigma_t = 0.0, friction_angle = 0.0, E = 0.0, Gt = 0.0, c_max = 0.0, c_threshold = 0.0;
	sigma_c = this->GetProperties()[YIELD_STRESS_C];
	sigma_t = this->GetProperties()[YIELD_STRESS_T];
	friction_angle = this->GetProperties()[INTERNAL_FRICTION_ANGLE] * 3.14159 / 180; // In radians!
	E = this->GetProperties()[YOUNG_MODULUS];
	Gt = this->GetProperties()[FRAC_ENERGY_T];
	c_max = std::abs(sigma_t);

	double ElementArea = this->GetGeometry().Area();
	//double l_char = sqrt(4 * ElementArea / sqrt(3));
	double A = 1.00 / (Gt * E / (l_char * std::pow(sigma_c, 2)) - 0.5);
	if (A < 0)
	{
		KRATOS_THROW_ERROR(std::invalid_argument, " 'A' damage parameter lower than zero --> Increase FRAC_ENERGY_T", A)
	}

	double f, F; /// F = f-c = 0 classical definition of yield surface
	f = GetMaxValue(PrincipalStressVector);

	if (this->GetThreshold(cont) == 0)
	{
		this->SetThreshold(c_max, cont);
	} // 1st iteration sets threshold as c_max
	c_threshold = this->GetThreshold(cont);
	this->Set_NonConvergedf_sigma(f, cont);

	F = f - c_threshold;

	if (F <= 0) // Elastic region --> Damage is constant
	{
		damage = this->Get_Convergeddamage();
		//this->Set_NonConvergeddamage(damage);
	}
	else
	{
		damage = 0.98; // Fragile  law
	}
	rIntegratedStress = StressVector;
	rIntegratedStress *= (1 - damage);
}

void FemDem2DElement::SetValueOnIntegrationPoints(
	const Variable<double> &rVariable,
	std::vector<double> &rValues,
	const ProcessInfo &rCurrentProcessInfo)
{
	for (unsigned int point_number = 0; point_number < GetGeometry().IntegrationPoints().size(); ++point_number)
	{
		this->SetValue(rVariable, rValues[point_number]);
	}
}

void FemDem2DElement::SetValueOnIntegrationPoints(
	const Variable<Vector> &rVariable,
	std::vector<Vector> &rValues,
	const ProcessInfo &rCurrentProcessInfo)
{
	for (unsigned int point_number = 0; point_number < GetGeometry().IntegrationPoints().size(); ++point_number)
	{
		this->SetValue(rVariable, rValues[point_number]);
	}
}

} // namespace Kratos