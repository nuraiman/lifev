SET(solver_HEADERS
  solver/VenantKirchhoffViscoelasticData.hpp
  solver/VenantKirchhoffElasticData.hpp
  solver/VenantKirchhoffSolverLinear.hpp
  solver/VenantKirchhoffElasticHandler.hpp
  solver/VenantKirchhoffSolver.hpp
  solver/VenantKirchhoffViscoelasticSolver.hpp
CACHE INTERNAL "")

SET(solver_SOURCES
  solver/VenantKirchhoffViscoelasticData.cpp
  solver/VenantKirchhoffElasticData.cpp
CACHE INTERNAL "")


SET(LOCAL_HEADERS)
FOREACH(INC ${solver_HEADERS})
  STRING(REPLACE "solver/" "" LOCAL_INC ${INC})
  SET(LOCAL_HEADERS ${LOCAL_HEADERS} ${LOCAL_INC})
ENDFOREACH()

INSTALL(
  FILES ${LOCAL_HEADERS}
  DESTINATION "${${PROJECT_NAME}_INSTALL_INCLUDE_DIR}/lifev/structure/solver"
  COMPONENT ${PACKAGE_NAME}
)
