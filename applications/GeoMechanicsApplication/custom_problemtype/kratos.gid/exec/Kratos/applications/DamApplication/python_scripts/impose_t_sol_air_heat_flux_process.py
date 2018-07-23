from KratosMultiphysics import *
from KratosMultiphysics.DamApplication import *

## In this case, the scalar value is automatically fixed.

def Factory(settings, Model):
    if(type(settings) != Parameters):
        raise Exception("expected input shall be a Parameters object, encapsulating a json string")
    return ImposeTSolAirHeatFluxProcess(Model, settings["Parameters"])

class ImposeTSolAirHeatFluxProcess(Process):
    
    def __init__(self, Model, settings ):

        Process.__init__(self)
        model_part = Model[settings["model_part_name"].GetString()]
        self.process = DamTSolAirHeatFluxProcess(model_part, settings) 
                 
    def ExecuteInitialize(self):
        self.process.ExecuteInitialize()

    def ExecuteInitializeSolutionStep(self):
        self.process.ExecuteInitializeSolutionStep()
