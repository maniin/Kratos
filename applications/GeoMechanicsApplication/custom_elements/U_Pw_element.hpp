//
//   Project Name:        KratosGeoMechanicsApplication $
//   Last Modified by:    $Author:    Ignasi de Pouplana $
//   Date:                $Date:           February 2016 $
//   Revision:            $Revision:                 1.0 $
//

#if !defined(KRATOS_U_PW_ELEMENT_H_INCLUDED )
#define  KRATOS_U_PW_ELEMENT_H_INCLUDED

// Project includes
#include "containers/array_1d.h"
#include "includes/define.h"
#include "includes/element.h"
#include "includes/serializer.h"
#include "geometries/geometry.h"
#include "utilities/math_utils.h"
#include "includes/constitutive_law.h"

// Application includes
#include "custom_utilities/element_utilities.hpp"
#include "geo_mechanics_application_variables.h"

namespace Kratos
{

template< unsigned int TDim, unsigned int TNumNodes >
class KRATOS_API(GEO_MECHANICS_APPLICATION) UPwElement : public Element
{

public:

    /// The definition of the sizetype
    typedef std::size_t SizeType;

    KRATOS_CLASS_POINTER_DEFINITION( UPwElement );

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    /// Default Constructor
    UPwElement(IndexType NewId = 0) : Element( NewId ) {}

    /// Constructor using an array of nodes
    UPwElement(IndexType NewId, const NodesArrayType& ThisNodes) : Element(NewId, ThisNodes) {}

    /// Constructor using Geometry
    UPwElement(IndexType NewId, GeometryType::Pointer pGeometry) : Element( NewId, pGeometry ) {}

    /// Constructor using Properties
    UPwElement(IndexType NewId, GeometryType::Pointer pGeometry, PropertiesType::Pointer pProperties) : Element( NewId, pGeometry, pProperties )
    {
        mThisIntegrationMethod = this->GetGeometry().GetDefaultIntegrationMethod();
    }

    /// Destructor
    virtual ~UPwElement() {}

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    Element::Pointer Create(IndexType NewId, NodesArrayType const& ThisNodes, PropertiesType::Pointer pProperties) const override;

    Element::Pointer Create(IndexType NewId, GeometryType::Pointer pGeom, PropertiesType::Pointer pProperties) const override;

    int Check(const ProcessInfo& rCurrentProcessInfo) override;

    void Initialize() override;

    void GetDofList( DofsVectorType& rElementalDofList, ProcessInfo& rCurrentProcessInfo ) override;

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    void CalculateLocalSystem(MatrixType& rLeftHandSideMatrix,VectorType& rRightHandSideVector,ProcessInfo& rCurrentProcessInfo ) override;

    void CalculateLeftHandSide(MatrixType& rLeftHandSideMatrix,ProcessInfo& rCurrentProcessInfo ) override;

    void CalculateRightHandSide(VectorType& rRightHandSideVector,ProcessInfo& rCurrentProcessInfo ) override;

    void EquationIdVector(EquationIdVectorType& rResult, ProcessInfo& rCurrentProcessInfo) override;

    void CalculateMassMatrix(MatrixType& rMassMatrix, ProcessInfo& rCurrentProcessInfo) override;

    void CalculateDampingMatrix(MatrixType& rDampingMatrix, ProcessInfo& rCurrentProcessInfo) override;

    void GetValuesVector(Vector& rValues, int Step = 0) override;

    void GetFirstDerivativesVector(Vector& rValues, int Step = 0) override;

    void GetSecondDerivativesVector(Vector& rValues, int Step = 0) override;

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    void SetValueOnIntegrationPoints(const Variable<double>& rVariable, std::vector<double>& rValues, const ProcessInfo& rCurrentProcessInfo) override;

    void GetValueOnIntegrationPoints(const Variable<double>& rVariable, std::vector<double>& rValues, const ProcessInfo& rCurrentProcessInfo) override;

    void GetValueOnIntegrationPoints(const Variable<array_1d<double,3>>& rVariable, std::vector<array_1d<double,3>>& rValues, const ProcessInfo& rCurrentProcessInfo) override;

    void GetValueOnIntegrationPoints(const Variable<Matrix>& rVariable, std::vector<Matrix>& rValues, const ProcessInfo& rCurrentProcessInfo) override;

    void GetValueOnIntegrationPoints( const Variable<ConstitutiveLaw::Pointer>& rVariable, std::vector<ConstitutiveLaw::Pointer>& rValues,const ProcessInfo& rCurrentProcessInfo ) override;

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

protected:

    /// Member Variables

    GeometryData::IntegrationMethod mThisIntegrationMethod;

    std::vector<ConstitutiveLaw::Pointer> mConstitutiveLawVector;

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

    virtual void CalculateStiffnessMatrix( MatrixType& rStiffnessMatrix, const ProcessInfo& CurrentProcessInfo );

    virtual void CalculateAll( MatrixType& rLeftHandSideMatrix, VectorType& rRightHandSideVector, const ProcessInfo& CurrentProcessInfo );

    virtual void CalculateRHS( VectorType& rRightHandSideVector, const ProcessInfo& CurrentProcessInfo );

    void CalculateIntegrationCoefficient(double& rIntegrationCoefficient, const double& detJ, const double& weight);

///----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

private:

    /// Serialization

    friend class Serializer;

    void save(Serializer& rSerializer) const override
    {
        KRATOS_SERIALIZE_SAVE_BASE_CLASS( rSerializer, Element )
    }

    void load(Serializer& rSerializer) override
    {
        KRATOS_SERIALIZE_LOAD_BASE_CLASS( rSerializer, Element )
    }

    /// Assignment operator.
    UPwElement & operator=(UPwElement const& rOther);

    /// Copy constructor.
    UPwElement(UPwElement const& rOther);


}; // Class UPwElement

} // namespace Kratos

#endif // KRATOS_U_PW_ELEMENT_H_INCLUDED  defined
