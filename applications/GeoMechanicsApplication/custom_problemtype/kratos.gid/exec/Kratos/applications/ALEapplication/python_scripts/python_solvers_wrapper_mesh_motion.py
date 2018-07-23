from __future__ import print_function, absolute_import, division #makes KratosMultiphysics backward compatible with python 2.6 and 2.7

import KratosMultiphysics

def CreateSolver(main_model_part, custom_settings):

    if (not isinstance(custom_settings, KratosMultiphysics.Parameters)):
        raise Exception("input is expected to be provided as a Kratos ModelPart object")

    if (not isinstance(custom_settings, KratosMultiphysics.Parameters)):
        raise Exception("input is expected to be provided as a Kratos Parameters object")

    parallelism = custom_settings["problem_data"]["parallel_type"].GetString()
    solver_type = custom_settings["solver_settings"]["solver_type"].GetString()

    # Solvers for OpenMP parallelism
    if (parallelism == "OpenMP"):

        if (solver_type == "mesh_solver_laplacian"):
            solver_module_name = "mesh_solver_laplacian"

        elif (solver_type == "mesh_solver_structural_similarity"):
            solver_module_name = "mesh_solver_structural_similarity"

        else:
            err_msg =  "The requested solver type \"" + solver_type + "\" is not in the python solvers wrapper\n"
            err_msg += "Available options are: \"mesh_solver_laplacian\", \"mesh_solver_structural_similarity\""
            raise Exception(err_msg)

    # Solvers for MPI parallelism
    elif (parallelism == "MPI"):

        if (solver_type == "mesh_solver_laplacian"):
            solver_module_name = "trilinos_mesh_solver_laplacian"

        elif (solver_type == "mesh_solver_structural_similarity"):
            solver_module_name = "trilinos_mesh_solver_structural_similarity"

        else:
            err_msg =  "The requested solver type \"" + solver_type + "\" is not in the python solvers wrapper\n"
            err_msg += "Available options are: \"mesh_solver_laplacian\", \"mesh_solver_structural_similarity\""
            raise Exception(err_msg)

    else:
        err_msg =  "The requested parallel type \"" + parallelism + "\" is not available!\n"
        err_msg += "Available options are: \"OpenMP\", \"MPI\""
        raise Exception(err_msg)

    solver_module = __import__(solver_module_name)
    solver = solver_module.CreateSolver(main_model_part, custom_settings["solver_settings"])

    return solver
