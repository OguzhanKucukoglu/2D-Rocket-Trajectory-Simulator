include("C:/Users/kucuk/Desktop/2D Trajectory Simulator/RocketSimulator/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/RocketSimulator-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "C:/Users/kucuk/Desktop/2D Trajectory Simulator/RocketSimulator/RocketSimulator.exe"
    GENERATE_QT_CONF
)
