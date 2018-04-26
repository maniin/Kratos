//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main author:     Ruben Zorrilla
//

#if !defined(KRATOS_EXPLICIT_MESH_MOVING_UTILITIES_H_INCLUDED )
#define  KRATOS_EXPLICIT_MESH_MOVING_UTILITIES_H_INCLUDED

// System includes
#include <string>
#include <iostream>

// External includes

// Project includes
#include "includes/define.h"
#include "includes/model_part.h"
#include "spatial_containers/spatial_search.h"
#include "spatial_containers/bins_dynamic_objects.h"
#include "spatial_containers/configures/node_configure.h"

namespace Kratos
{
  ///@addtogroup FluidDynamicsApplication
  ///@{

  ///@name Kratos Globals
  ///@{

  ///@}
  ///@name Type Definitions
  ///@{

  ///@}
  ///@name  Enum's
  ///@{

  ///@}
  ///@name  Functions
  ///@{

  ///@}
  ///@name Kratos Classes
  ///@{

  /// Utility to compute FILL INFO 
  /** TODO: FILL INFO
   * TODO: FILL INFO
   */
  class KRATOS_API(ALE_APPLICATION) ExplicitMeshMovingUtilities
  {
  public:

    ///@name Type Definitions
    ///@{

    typedef BinsObjectDynamic<NodeConfigure>                                      NodeBinsType;
    typedef SpatialSearch::DistanceType                                           DistanceType;
    typedef SpatialSearch::ResultNodesContainerType                   ResultNodesContainerType;
    typedef SpatialSearch::VectorDistanceType                               VectorDistanceType;
    typedef SpatialSearch::VectorResultNodesContainerType       VectorResultNodesContainerType;

    /// Pointer definition of ExplicitMeshMovingUtilities
    KRATOS_CLASS_POINTER_DEFINITION(ExplicitMeshMovingUtilities);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Constructor
    ExplicitMeshMovingUtilities(
        ModelPart &rModelPart,
        ModelPart &rStructureModelPart,
        const double SearchRadius);

    /// Destructor.
    ~ExplicitMeshMovingUtilities() {};

    ///@}
    ///@name Operators
    ///@{


    ///@}
    ///@name Operations
    ///@{

    /**
    * According to mSearchRadius, performs the bins search of the close structure nodes for each fluid node
    * @return rSearchResults vector containing the the rStructureModelPart nodes inside 
    * @return rSearchDistanceResults vector containing the the rStructureModelPart nodes 
    * inside SearchRadius distance values for each rModelPart nodes
    */
    void SearchStructureNodes(
        VectorResultNodesContainerType &rSearchResults,
        VectorDistanceType &rSearchDistanceResults);

    /**
    * Computes the MESH_DISPLACEMENT value for each fluid node. This operation is explicitly computed 
    * as a weighted average of the structure nodes DISPLACEMENT values within the mSearchRadius. The 
    * weights are computed using a kernel function.
    * @param rSearchResults vector containing the the rStructureModelPart nodes inside 
    * @param rSearchDistanceResults vector containing the the rStructureModelPart nodes 
    * inside SearchRadius distance values for each rModelPart nodes
    */
    void ComputeMeshDisplacement(
        const VectorResultNodesContainerType &rSearchResults,
        const VectorDistanceType &rSearchDistanceResults);

    /**
    * Computes the kernel function value for a given normalised distance value
    * @param NormalisedDistance structure node distance value normalised with the mSearchRadius
    */
    inline double ComputeKernelValue(const double NormalisedDistance);

    /**
    * This method performs the explicit mesh movement (computes the MESH_DISPLACEMENT value and moves
    * the mesh accordingly) and computes the MESH_VELOCITY values.
    * @param DeltaTime time step value (used in the computation of the MESH_VELOCITY values)
    */
    void ComputeExplicitMeshMovement(const double DeltaTime);

    ///@}
    ///@name Access
    ///@{


    ///@}
    ///@name Inquiry
    ///@{


    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const;

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const;

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const;

    ///@}
    ///@name Friends
    ///@{


    ///@}

private:
    ///@name Static Member Variables
    ///@{


    ///@}
    ///@name Member Variables
    ///@{

    const double mSearchRadius;

    ModelPart &mrModelPart;
    ModelPart &mrStructureModelPart;

    ///@}
    ///@name Private Operators
    ///@{


    ///@}
    ///@name Private Operations
    ///@{


    ///@}
    ///@name Private  Access
    ///@{


    ///@}
    ///@name Private Inquiry
    ///@{


    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
    ExplicitMeshMovingUtilities& operator=(ExplicitMeshMovingUtilities const& rOther);

    /// Copy constructor.
    ExplicitMeshMovingUtilities(ExplicitMeshMovingUtilities const& rOther);

    ///@}

}; // Class ExplicitMeshMovingUtilities

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{


/// output stream function
inline std::ostream& operator << (
    std::ostream& rOStream,
    const ExplicitMeshMovingUtilities& rThis);

///@}

///@} addtogroup block

}  // namespace Kratos.

#endif // KRATOS_EXPLICIT_MESH_MOVING_UTILITIES_H_INCLUDED  defined
