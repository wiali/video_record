include($$(FORTIS_SDK)/fortis_sdk.pri)

requirePackage(settings, 2.0.1)
requirePackage(spdlog, 1.0.1)
requirePackage(logger, 2.0.4)
requirePackage(opencv, 3.2.0)

addToDeployList(opencv, 3.2.0)
