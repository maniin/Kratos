# import Kratos
import KratosMultiphysics
import KratosMultiphysics.SolidMechanicsApplication
import KratosMultiphysics.ConstitutiveModelsApplication

# Import Kratos "wrapper" for unittests
import KratosMultiphysics.KratosUnittest as KratosUnittest

# Import the tests o test_classes to create the suits:

# CORE TESTS
import CoreTests
# SMALL TESTS
import SmallTests
# NIGTHLY TESTS
import NightTests
# VALIDATION TESTS
import ValidationTests

def AssambleTestSuites():

    # Suites to run
    suites = KratosUnittest.KratosSuites

    # CORE TESTS
    CoreTests.SetTestSuite(suites)
    
    # SMALL TESTS
    small_suite = SmallTests.SetTestSuite(suites)

    # NIGTHLY TESTS
    night_suite = NightTests.SetTestSuite(suites)

    # inlude small suite in night suite
    night_suite.addTests(small_suite)

    # VALIDATION TESTS
    validation_suite = ValidationTests.SetTestSuite(suites)

    # ALL TESTS
    all_suite = suites['all']

    #all_suite.addTests(night_suite)
    #all_suite.addTests(validation_suite)

    return suites

if __name__ == '__main__':
    KratosUnittest.runTests(AssambleTestSuites())
