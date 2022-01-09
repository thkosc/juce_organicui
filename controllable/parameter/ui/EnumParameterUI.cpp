/*
  ==============================================================================

	EnumParameterUI.cpp
	Created: 29 Sep 2016 5:35:12pm
	Author:  bkupe

  ==============================================================================
*/


EnumParameterUI::EnumParameterUI(Array<EnumParameter*> parameters) :
	ParameterUI(Inspectable::getArrayAs<EnumParameter, Parameter>(parameters)),
	eps(parameters),
	ep(parameters[0])
{
	cb.addListener(this);
	cb.setTextWhenNoChoicesAvailable("");
	cb.setTextWhenNothingSelected("Select an element");
	cb.setTooltip(ep->description);
	addAndMakeVisible(cb);
	
	ep->addAsyncEnumParameterListener(this);

	cb.addMouseListener(this, true);

	prevValue = ep->getValueKey();

	showEditWindowOnDoubleClick = false;

	updateUIParams();
}

EnumParameterUI::~EnumParameterUI()
{
	if (!parameter.wasObjectDeleted()) ep->removeAsyncEnumParameterListener(this);
	cb.removeListener(this);
}

void EnumParameterUI::updateComboBox()
{
	cb.clear(dontSendNotification);
	idKeyMap.clear();

	if (parameter.wasObjectDeleted()) return;

	int id = 1;
	for (auto& ev : ep->enumValues)
	{
		cb.addItem(ev->key, id);
		idKeyMap.set(id, ev->key);
		keyIdMap.set(ev->key, id);
		id++;
	}

	cb.setSelectedId(keyIdMap[ep->getValueKey()], dontSendNotification);
	cb.setEnabled(isInteractable());

	updateTooltip();
}

String EnumParameterUI::getSelectedKey()
{
	return idKeyMap[cb.getSelectedId()];
}

void EnumParameterUI::resized()
{
	cb.setBounds(getLocalBounds());
}

void EnumParameterUI::newMessage(const EnumParameter::EnumParameterEvent& e)
{
	updateComboBox();
}

void EnumParameterUI::updateUIParamsInternal()
{
	Colour bgColor = useCustomBGColor ? customBGColor : BG_COLOR;
	Colour fgColor = useCustomTextColor ? customTextColor : TEXT_COLOR;

	cb.setColour(cb.backgroundColourId, bgColor);
	cb.setColour(cb.outlineColourId, bgColor.brighter(.2f));
	cb.setColour(cb.focusedOutlineColourId, bgColor.brighter(.3f));
	cb.setColour(cb.textColourId, fgColor);
	cb.setColour(cb.buttonColourId, fgColor);
	cb.setColour(cb.arrowColourId, fgColor.darker(.2f));
	updateComboBox();
}

void EnumParameterUI::valueChanged(const var& value)
{
	cb.setSelectedId(keyIdMap[ep->getValueKey()], dontSendNotification);
	prevValue = ep->getValueKey();
}

void EnumParameterUI::comboBoxChanged(ComboBox*)
{
	if (shouldBailOut()) return;
	ep->setUndoableValue(prevValue, getSelectedKey());

}
void EnumParameterUI::addPopupMenuItemsInternal(PopupMenu* p)
{
	p->addItem(101, "Set Options...");
}

void EnumParameterUI::handleMenuSelectedID(int result)
{
	ParameterUI::handleMenuSelectedID(result);
	if (result == 101)
	{
		std::unique_ptr<Component> editComponent(new EnumOptionManager(ep));
		CallOutBox* box = &CallOutBox::launchAsynchronously(std::move(editComponent), localAreaToGlobal(getLocalBounds()), nullptr);
		box->setArrowSize(8);
	}
}

EnumParameterUI::EnumOptionManager::EnumOptionManager(EnumParameter* ep) :
	ep(ep)
{
	int numRowsToDisplay = ep->enumValues.size() + 5;
	for (int i = 0; i < numRowsToDisplay; i++)
	{
		EnumOptionUI* ui = new EnumOptionUI(ep, i);
		optionsUI.add(ui);
		ui->keyLabel.addListener(this);
		ui->valueLabel.addListener(this);
		container.addAndMakeVisible(ui);
	}

	viewport.setScrollBarsShown(true, false);
	addAndMakeVisible(&viewport);
	viewport.setViewedComponent(&container, false);

	setSize(200, 140);
}

EnumParameterUI::EnumOptionManager::~EnumOptionManager()
{
}

void EnumParameterUI::EnumOptionManager::paint(Graphics& g)
{
	Rectangle<int> hr = getLocalBounds().removeFromTop(20);
	g.setColour(TEXT_COLOR);
	g.drawText("Value", hr.removeFromRight(getWidth() / 2).reduced(2).toFloat(), Justification::centred, false);
	g.drawText("Key", hr.reduced(2).toFloat(), Justification::centred, false);
}

void EnumParameterUI::EnumOptionManager::resized()
{
	Rectangle<int> r = getLocalBounds().withHeight(20);
	for (int i = 0; i < optionsUI.size(); i++) optionsUI[i]->setBounds(r.translated(0, i * r.getHeight()));

	int th = optionsUI.size() * r.getHeight();
	container.setSize(getWidth() - 10, th);
	viewport.setBounds(getLocalBounds().withTrimmedTop(20));
}

void EnumParameterUI::EnumOptionManager::labelTextChanged(Label* l)
{
	String key = ep->getValueKey();

	ep->clearOptions();

	for (auto& o : optionsUI)
	{
		String k = o->keyLabel.getText();
		String v = o->valueLabel.getText();
		if (k.isEmpty() || v.isEmpty()) continue;
		ep->addOption(k, v, false);
	}

	ep->setValueWithKey(key);
}

EnumParameterUI::EnumOptionManager::EnumOptionUI::EnumOptionUI(EnumParameter* ep, int index) :
	ep(ep),
	index(index)
{
	if (index < ep->enumValues.size())
	{
		keyLabel.setText(ep->enumValues[index]->key, dontSendNotification);
		valueLabel.setText(ep->enumValues[index]->value.toString(), dontSendNotification);
	}


	keyLabel.setEditable(true);
	keyLabel.setBorderSize(BorderSize<int>(1));
	keyLabel.setColour(Label::backgroundColourId, BG_COLOR);
	keyLabel.setColour(Label::textColourId, TEXT_COLOR.darker(.2f));
	keyLabel.setColour(Label::textWhenEditingColourId, TEXT_COLOR);

	valueLabel.setEditable(true);
	valueLabel.setBorderSize(BorderSize<int>(1));
	valueLabel.setColour(Label::backgroundColourId, BG_COLOR);
	valueLabel.setColour(Label::textColourId, TEXT_COLOR.darker(.2f));
	valueLabel.setColour(Label::textWhenEditingColourId, TEXT_COLOR);


	addAndMakeVisible(&keyLabel);
	addAndMakeVisible(&valueLabel);
}

void EnumParameterUI::EnumOptionManager::EnumOptionUI::resized()
{
	keyLabel.setBounds(getLocalBounds().removeFromLeft(getWidth() / 2).reduced(2));
	valueLabel.setBounds(getLocalBounds().removeFromRight(getWidth() / 2).reduced(2));
}