QT += widgets xml

DEFINES += TWO_D_MODEL_LIBRARY

links(qrkernel qrutils qrgui-tool-plugin-interface robots-utils robots-kit-base)
includes(plugins/robots/common/kitBase)
includes(plugins/robots/utils qrtext)

TRANSLATIONS = $$PWD/../../../../qrtranslations/ru/plugins/robots/twoDModel_ru.ts

HEADERS += \
	$$PWD/include/twoDModel/twoDModelDeclSpec.h \
	$$PWD/include/twoDModel/engine/twoDModelControlInterface.h \
	$$PWD/include/twoDModel/engine/twoDModelDisplayInterface.h \
	$$PWD/include/twoDModel/engine/twoDModelDisplayWidget.h \
	$$PWD/include/twoDModel/engine/twoDModelEngineFacade.h \
	$$PWD/include/twoDModel/engine/twoDModelEngineInterface.h \
	$$PWD/include/twoDModel/engine/view/d2ModelWidget.h \
	$$PWD/include/twoDModel/robotModel/twoDRobotModel.h \
	$$PWD/include/twoDModel/robotModel/parts/button.h \
	$$PWD/include/twoDModel/robotModel/parts/colorSensorBlue.h \
	$$PWD/include/twoDModel/robotModel/parts/colorSensorFull.h \
	$$PWD/include/twoDModel/robotModel/parts/colorSensorGreen.h \
	$$PWD/include/twoDModel/robotModel/parts/colorSensorPassive.h \
	$$PWD/include/twoDModel/robotModel/parts/colorSensorRed.h \
	$$PWD/include/twoDModel/robotModel/parts/display.h \
	$$PWD/include/twoDModel/robotModel/parts/encoderSensor.h \
	$$PWD/include/twoDModel/robotModel/parts/lightSensor.h \
	$$PWD/include/twoDModel/robotModel/parts/motor.h \
	$$PWD/include/twoDModel/robotModel/parts/rangeSensor.h \
	$$PWD/include/twoDModel/robotModel/parts/touchSensor.h \
	$$PWD/include/twoDModel/robotModel/parts/gyroscope.h \
	$$PWD/include/twoDModel/robotModel/parts/accelerometer.h \
	$$PWD/include/twoDModel/robotModel/nullTwoDRobotModel.h \
	$$PWD/include/twoDModel/engine/nullTwoDModelDisplayWidget.h \
	$$PWD/include/twoDModel/robotModel/parts/marker.h \
	$$PWD/include/twoDModel/blocks/markerDownBlock.h \
	$$PWD/include/twoDModel/blocks/markerUpBlock.h \

HEADERS += \
	$$PWD/src/engine/twoDModelEngineApi.h \
	$$PWD/src/engine/view/d2ModelScene.h \
	$$PWD/src/engine/view/robotItem.h \
	$$PWD/src/engine/view/sensorItem.h \
	$$PWD/src/engine/view/sonarSensorItem.h \
	$$PWD/src/engine/view/gridParameters.h \
	$$PWD/src/engine/constraints/constraintsChecker.h \
	$$PWD/src/engine/constraints/details/defines.h \
	$$PWD/src/engine/constraints/details/constraintsParser.h \
	$$PWD/src/engine/constraints/details/event.h \
	$$PWD/src/engine/constraints/details/conditionsFactory.h \
	$$PWD/src/engine/constraints/details/triggersFactory.h \
	$$PWD/src/engine/constraints/details/valuesFactory.h \
	$$PWD/src/engine/model/model.h \
	$$PWD/src/engine/model/constants.h \
	$$PWD/src/engine/model/settings.h \
	$$PWD/src/engine/model/sensorsConfiguration.h \
	$$PWD/src/engine/model/worldModel.h \
	$$PWD/src/engine/model/timeline.h \
	$$PWD/src/engine/model/modelTimer.h \
	$$PWD/src/engine/model/robotModel.h \
	$$PWD/src/engine/model/physics/physicsEngineBase.h \
	$$PWD/src/engine/model/physics/simplePhysicsEngine.h \
	$$PWD/src/engine/model/physics/realisticPhysicsEngine.h \
	$$PWD/src/engine/items/wallItem.h \
	$$PWD/src/engine/items/stylusItem.h \
	$$PWD/src/engine/items/lineItem.h \
	$$PWD/src/engine/items/colorFieldItem.h \
	$$PWD/src/engine/items/ellipseItem.h \
	$$PWD/src/engine/items/startPosition.h \
	$$PWD/src/engine/items/regions/regionItem.h \
	$$PWD/src/engine/items/regions/ellipseRegion.h \
	$$PWD/src/engine/items/regions/rectangularRegion.h \
	$$PWD/src/engine/items/regions/boundRegion.h \

SOURCES += \
	$$PWD/src/engine/twoDModelEngineFacade.cpp \
	$$PWD/src/engine/twoDModelEngineApi.cpp \
	$$PWD/src/engine/view/d2ModelWidget.cpp \
	$$PWD/src/engine/view/d2ModelScene.cpp \
	$$PWD/src/engine/view/robotItem.cpp \
	$$PWD/src/engine/view/sensorItem.cpp \
	$$PWD/src/engine/view/sonarSensorItem.cpp \
	$$PWD/src/engine/view/gridParameters.cpp \
	$$PWD/src/engine/view/twoDModelDisplayWidget.cpp \
	$$PWD/src/engine/model/model.cpp \
	$$PWD/src/engine/model/settings.cpp \
	$$PWD/src/engine/model/robotModel.cpp \
	$$PWD/src/engine/model/modelTimer.cpp \
	$$PWD/src/engine/model/sensorsConfiguration.cpp \
	$$PWD/src/engine/model/worldModel.cpp \
	$$PWD/src/engine/model/timeline.cpp \
	$$PWD/src/engine/model/physics/physicsEngineBase.cpp \
	$$PWD/src/engine/model/physics/simplePhysicsEngine.cpp \
	$$PWD/src/engine/model/physics/realisticPhysicsEngine.cpp \
	$$PWD/src/engine/items/wallItem.cpp \
	$$PWD/src/engine/items/stylusItem.cpp \
	$$PWD/src/engine/items/lineItem.cpp \
	$$PWD/src/engine/items/colorFieldItem.cpp \
	$$PWD/src/engine/items/ellipseItem.cpp \
	$$PWD/src/engine/items/startPosition.cpp \
	$$PWD/src/engine/items/regions/regionItem.cpp \
	$$PWD/src/engine/items/regions/ellipseRegion.cpp \
	$$PWD/src/engine/items/regions/rectangularRegion.cpp \
	$$PWD/src/engine/items/regions/boundRegion.cpp \
	$$PWD/src/engine/constraints/constraintsChecker.cpp \
	$$PWD/src/engine/constraints/details/constraintsParser.cpp \
	$$PWD/src/engine/constraints/details/event.cpp \
	$$PWD/src/engine/constraints/details/conditionsFactory.cpp \
	$$PWD/src/engine/constraints/details/triggersFactory.cpp \
	$$PWD/src/engine/constraints/details/valuesFactory.cpp \
	$$PWD/src/robotModel/twoDRobotModel.cpp \
	$$PWD/src/robotModel/parts/button.cpp \
	$$PWD/src/robotModel/parts/colorSensorBlue.cpp \
	$$PWD/src/robotModel/parts/colorSensorFull.cpp \
	$$PWD/src/robotModel/parts/colorSensorGreen.cpp \
	$$PWD/src/robotModel/parts/colorSensorPassive.cpp \
	$$PWD/src/robotModel/parts/colorSensorRed.cpp \
	$$PWD/src/robotModel/parts/display.cpp \
	$$PWD/src/robotModel/parts/encoderSensor.cpp \
	$$PWD/src/robotModel/parts/lightSensor.cpp \
	$$PWD/src/robotModel/parts/motor.cpp \
	$$PWD/src/robotModel/parts/rangeSensor.cpp \
	$$PWD/src/robotModel/parts/touchSensor.cpp \
	$$PWD/src/robotModel/parts/gyroscope.cpp \
	$$PWD/src/robotModel/parts/accelerometer.cpp \
	$$PWD/src/engine/nullTwoDModelDisplayWidget.cpp \
	$$PWD/src/robotModel/nullTwoDRobotModel.cpp \
	$$PWD/src/robotModel/parts/marker.cpp \
	$$PWD/src/blocks/markerDownBlock.cpp \
	$$PWD/src/blocks/markerUpBlock.cpp \

FORMS += \
	$$PWD/src/engine/view/d2Form.ui \
