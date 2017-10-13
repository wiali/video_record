include($$(FORTIS_SDK)/fortis_sdk.pri)

requirePackage(settings, 2.0.1)
requirePackage(geometry, 0.2.4)
requirePackage(segmentation_data, 0.3.4)
requirePackage(segmentation, 2.2.1)
requirePackage(sensor_data, 1.3.1)
requirePackage(calibration_data, 1.1.1)

addToDeployList(geometry, 0.2.4)
addToDeployList(segmentation_data, 0.3.4)
addToDeployList(segmentation, 2.2.1)
addToDeployList(sensor_data, 1.3.1)
addToDeployList(calibration_data, 1.1.1)
