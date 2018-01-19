#include "TargetParameterUI.h"
/*
  ==============================================================================

	TargetParameterUI.cpp
	Created: 2 Nov 2016 5:00:10pm
	Author:  bkupe

  ==============================================================================
*/


TargetParameterUI::TargetParameterUI(TargetParameter * parameter, const String &_noTargetText) :
	ParameterUI(parameter),
	noTargetText(_noTargetText),
	targetParameter(parameter)
{
	setInterceptsMouseClicks(true, true);
	showEditWindowOnDoubleClick = false;
	showEditWindowOnRightClick = false;

	targetBT = AssetManager::getInstance()->getTargetBT();
	targetBT->setInterceptsMouseClicks(false, false);

	if (targetParameter->customCheckAssignOnNextChangeFunc != nullptr)
	{
		
		listeningToNextChange = new BoolParameter("Auto Learn", "When this parameter is on, any parameter that changes will be auto assigned to this target", false);
		listeningToNextChange->addAsyncParameterListener(this);
		listeningToNextChangeBT = listeningToNextChange->createToggle();
		addAndMakeVisible(listeningToNextChangeBT);
	}


	addAndMakeVisible(targetBT);

	targetBT->addListener(this);
	setRepaintsOnMouseActivity(true);

	label.setFont(label.getFont().withHeight(12));
	updateLabel();
	label.setInterceptsMouseClicks(false, false);
	addAndMakeVisible(label);
}


TargetParameterUI::~TargetParameterUI()
{
	//in case we deleted with the listener still on
	if(parameter != nullptr && !parameter.wasObjectDeleted()) targetParameter->rootContainer->removeControllableContainerListener(this);
}

void TargetParameterUI::paint(Graphics & g)
{

	Colour c = targetParameter->target != nullptr || targetParameter->targetContainer != nullptr ? GREEN_COLOR : NORMAL_COLOR;
	if (isMouseOver()) c = c.brighter();

	g.setGradientFill(ColourGradient(c.brighter(), (float)getLocalBounds().getCentreX(), (float)getLocalBounds().getCentreY(), c.darker(), 2.f, 2.f, true));
	g.fillRoundedRectangle(targetBT->getBounds().expanded(2).toFloat(), 2);

}

void TargetParameterUI::resized()
{
	Rectangle<int> r = getLocalBounds();
	if (listeningToNextChangeBT != nullptr)
	{
		listeningToNextChangeBT->setBounds(r.removeFromRight(100));
		r.removeFromRight(2);
	}

	targetBT->setBounds(r.removeFromLeft(r.getHeight()).reduced(2));
	r.removeFromLeft(2);
	label.setBounds(r.reduced(0, 2));
}

void TargetParameterUI::updateLabel()
{
	String newText;
	if (targetParameter->targetType == TargetParameter::TargetType::CONTROLLABLE)
	{
		if (targetParameter->target != nullptr)
		{
			if (targetParameter->customGetControllableLabelFunc != nullptr) newText = targetParameter->customGetControllableLabelFunc(targetParameter->target);
			else newText = targetParameter->showFullAddressInEditor ? targetParameter->target->getControlAddress() : (targetParameter->showParentNameInEditor ? targetParameter->target->parentContainer->niceName + ":" : "") + targetParameter->target->niceName;
		}
	} else //TargetType::CONTAINER
	{
		if (targetParameter->targetContainer != nullptr)
		{
			if (targetParameter->customGetContainerLabelFunc != nullptr) newText = targetParameter->customGetContainerLabelFunc(targetParameter->targetContainer);
			else newText = newText = targetParameter->showFullAddressInEditor ? targetParameter->targetContainer->getControlAddress() : (targetParameter->showParentNameInEditor ? targetParameter->targetContainer->parentContainer->niceName + ":" : "") + targetParameter->targetContainer->niceName;
		}
	}

	if (newText.isEmpty())
	{
		if (targetParameter->ghostValue.isNotEmpty()) newText = "### " + targetParameter->ghostValue;
		else newText = noTargetText;
	}

	label.setText(newText, dontSendNotification);
}

void TargetParameterUI::showPopupAndGetTarget()
{
	if (!parameter->enabled) return;

	if (targetParameter->targetType == TargetParameter::TargetType::CONTROLLABLE)
	{
		Controllable * c = nullptr;

		if (targetParameter->customGetTargetFunc != nullptr)
		{
			c = targetParameter->customGetTargetFunc(targetParameter->showTriggers,targetParameter->showParameters);
		} else
		{
			ControllableChooserPopupMenu p(targetParameter->rootContainer, targetParameter->showParameters, targetParameter->showTriggers);
			c = p.showAndGetControllable();
		}
		if (c != nullptr) targetParameter->setValueFromTarget(c);

	} else
	{
		ControllableContainer * cc = nullptr;
		if (targetParameter->customGetTargetContainerFunc != nullptr)
		{
			cc = targetParameter->customGetTargetContainerFunc();
		}
		if (cc != nullptr) targetParameter->setValueFromTarget(cc);
	}
}

void TargetParameterUI::mouseDownInternal(const MouseEvent &)
{
	if (!targetParameter->isEditable || forceFeedbackOnly) return;

	showPopupAndGetTarget();
}

void TargetParameterUI::buttonClicked(Button * b)
{
	if (b == targetBT) {} // move code here ?
}

void TargetParameterUI::valueChanged(const var &)
{
	DBG("Value changed ");
	if (listeningToNextChange != nullptr) listeningToNextChange->setValue(false);

	updateLabel();
	repaint();
}

void TargetParameterUI::newMessage(const Parameter::ParameterEvent & e)
{

	if (e.parameter == listeningToNextChange)
	{
		if (e.type == Parameter::ParameterEvent::VALUE_CHANGED)
		{
			{
				if (listeningToNextChange->boolValue())
				{
					targetParameter->rootContainer->addControllableContainerListener(this);
				} else
				{
					targetParameter->rootContainer->removeControllableContainerListener(this);
				}
			}
		}
	} else
	{
		ParameterUI::newMessage(e);
	}
	
}

void TargetParameterUI::controllableFeedbackUpdate(ControllableContainer *, Controllable * c)
{
	if (c == targetParameter->target) return;

	if (c->type == Controllable::TRIGGER)
	{
		if (!targetParameter->showTriggers) return;
	} else
	{
		if (!targetParameter->showParameters) return;
	}

	bool isControllableValid = targetParameter->customCheckAssignOnNextChangeFunc(c);
	if (isControllableValid) targetParameter->setValueFromTarget(c);
}
