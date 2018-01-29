//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Aditya Ghantasala
//

#if !defined(KRATOS_COSIMULATION_BASE_APPLICATION_H_INCLUDED)
#define KRATOS_COSIMULATION_BASE_APPLICATION_H_INCLUDED

// System includes
#include <string>
#include <iostream>

// Project includes

// Application includes
#include "custom_base_classes/base_co_simulation_application_io.h"
#include "custom_base_classes/base_co_simulation_data.h"
#include "custom_base_classes/base_co_simulation_mesh.h"

class CoSimulationBaseApplication
{

  public:
    ///@name Type Definitions
    ///@{
    typedef std::shared_ptr<CoSimulationBaseApplication> Pointer;
    typedef CoSimulationData<double>::Pointer DataPointerType;
    typedef CoSimulationBaseIo::Pointer BaseIoPointerType;
    typedef CoSimulationMesh::Pointer MeshPointerType;
    ///@}
    ///@name Life Cycle
    ///@{
    CoSimulationBaseApplication(std::string iName) : mName(iName)
    {
    }

    virtual ~CoSimulationBaseApplication()
    {
    }

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    std::string Name() { return this->mName; }

    /////////////////////////////////////////////////
    /// Methods specific for Co-Simulation
    /////////////////////////////////////////////////

    virtual DataPointerType AddCoSimulationData(std::string iName, unsigned int iSize)
    {
        DataPointerType data = DataPointerType(new CoSimulationData<double>(iName, iSize));
        mCoSimulationData.push_back(data);
        return data;
    }


    virtual void SetIo(BaseIoPointerType iIo)
    {
        mpIo = iIo;
    }

    /// Data synchronization methods
    virtual void ExportData(DataPointerType iData, Pointer iToApp)
    {
        mpIo->ExportData(iData, this->Name(), iToApp->Name());
    }

    virtual void ImportData(DataPointerType iData, Pointer iFromApp)
    {
        mpIo->ImportData(iData, iFromApp->Name(), this->Name());
    }

    virtual void MakeDataAvailable(DataPointerType iData, Pointer iToApp)
    {
        mpIo->MakeDataAvailable(iData, this->Name(), iToApp->Name());
    }

    virtual void ExportMesh(MeshPointerType iMesh, Pointer iToApp)
    {
        mpIo->ExportMesh(iMesh, this->Name(), iToApp->Name());
    }

    virtual void ImportMesh(MeshPointerType iMesh, Pointer iFromApp)
    {
        mpIo->ImportMesh(iMesh, iFromApp->Name(), this->Name());
    }

    virtual void MakeMeshAvailable(MeshPointerType iMesh, Pointer iToApp)
    {
        mpIo->MakeMeshAvailable(iMesh, this->Name(), iToApp->Name());
    }

    ///@}
    ///@name Access
    ///@{

    ///@}
    ///@name Inquiry
    ///@{

    ///@}
    ///@name Input and output
    ///@{

    ///@}
    ///@name Friends
    ///@{

    ///@}

  protected:
    ///@name Protected static Member Variables
    ///@{

    ///@}
    ///@name Protected member Variables
    ///@{

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Protected Operations
    ///@{

    ///@}
    ///@name Protected  Access
    ///@{

    ///@}
    ///@name Protected Inquiry
    ///@{

    ///@}
    ///@name Protected LifeCycle
    ///@{

    ///@}

  private:
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{
    std::string mName;
    BaseIoPointerType mpIo;
    std::vector<DataPointerType> mCoSimulationData;
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

    ///@}

}; // End class

#endif // KRATOS_CO_SIMULATION_APPLICATION_H_INCLUDED  defined