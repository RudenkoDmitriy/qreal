HEADERS += \
	$$PWD/smartLine.h \
	$$PWD/generatorCustomizer.h \
	$$PWD/controlFlow.h \
	$$PWD/controlFlowGeneratorBase.h \
	$$PWD/readableControlFlowGenerator.h \
	$$PWD/gotoControlFlowGenerator.h \
	$$PWD/robotsDiagramVisitor.h \
	$$PWD/primaryControlFlowValidator.h \
	$$PWD/semanticTree/semanticTree.h \
	$$PWD/semanticTree/semanticNode.h \
	$$PWD/semanticTree/nonZoneNode.h \
	$$PWD/semanticTree/zoneNode.h \
	$$PWD/semanticTree/simpleNode.h \
	$$PWD/semanticTree/conditionalNode.h \
	$$PWD/semanticTree/ifNode.h \
	$$PWD/semanticTree/loopNode.h \
	$$PWD/semanticTree/rootNode.h \
	$$PWD/rules/semanticTransformationRule.h \
	$$PWD/rules/simpleRules/simpleBlockRuleBase.h \
	$$PWD/rules/simpleRules/simpleUnvisitedRule.h \
	$$PWD/rules/simpleRules/simpleVisitedOneZoneRule.h \
	$$PWD/rules/simpleRules/simpleVisitedRuleBase.h \
	$$PWD/rules/simpleRules/simpleMergedIfBranchesRule.h \
	$$PWD/rules/simpleRules/simpleIfInsideCycleRule.h \
	$$PWD/rules/ifRules/ifBlockRuleBase.h \
	$$PWD/rules/ifRules/ifWithBothUnvisitedRule.h \
	$$PWD/rules/ifRules/ifWithOneVisitedRule.h \
	$$PWD/rules/loopRules/loopBlockRuleBase.h \
	$$PWD/rules/loopRules/loopWithBothUnvisitedRule.h \

SOURCES += \
	$$PWD/smartLine.cpp \
	$$PWD/generatorCustomizer.cpp \
	$$PWD/controlFlow.cpp \
	$$PWD/controlFlowGeneratorBase.cpp \
	$$PWD/readableControlFlowGenerator.cpp \
	$$PWD/gotoControlFlowGenerator.cpp \
	$$PWD/robotsDiagramVisitor.cpp \
	$$PWD/primaryControlFlowValidator.cpp \
	$$PWD/semanticTree/semanticTree.cpp \
	$$PWD/semanticTree/semanticNode.cpp \
	$$PWD/semanticTree/nonZoneNode.cpp \
	$$PWD/semanticTree/zoneNode.cpp \
	$$PWD/semanticTree/simpleNode.cpp \
	$$PWD/semanticTree/conditionalNode.cpp \
	$$PWD/semanticTree/ifNode.cpp \
	$$PWD/semanticTree/loopNode.cpp \
	$$PWD/semanticTree/rootNode.cpp \
	$$PWD/rules/semanticTransformationRule.cpp \
	$$PWD/rules/simpleRules/simpleBlockRuleBase.cpp \
	$$PWD/rules/simpleRules/simpleUnvisitedRule.cpp \
	$$PWD/rules/simpleRules/simpleVisitedOneZoneRule.cpp \
	$$PWD/rules/simpleRules/simpleVisitedRuleBase.cpp \
	$$PWD/rules/simpleRules/simpleMergedIfBranchesRule.cpp \
	$$PWD/rules/simpleRules/simpleIfInsideCycleRule.cpp \
	$$PWD/rules/ifRules/ifBlockRuleBase.cpp \
	$$PWD/rules/ifRules/ifWithBothUnvisitedRule.cpp \
	$$PWD/rules/ifRules/ifWithOneVisitedRule.cpp \
	$$PWD/rules/loopRules/loopBlockRuleBase.cpp \
	$$PWD/rules/loopRules/loopWithBothUnvisitedRule.cpp \
