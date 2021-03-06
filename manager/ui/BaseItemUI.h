/*
  ==============================================================================

	BaseItemUI.h
	Created: 28 Oct 2016 8:04:09pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

template<class T>
class BaseItemUI :
	public BaseItemMinimalUI<T>,
	public Button::Listener, 
	public Label::Listener,
	public ComponentListener
{
public:
	enum Direction { NONE, VERTICAL, HORIZONTAL, ALL };

	BaseItemUI<T>(T * _item, Direction resizeDirection = NONE);
	virtual ~BaseItemUI<T>();

	//LAYOUT
	int margin;
	int minContentHeight;


	Point<float> sizeAtMouseDown;

	//header
	int headerHeight;
	int headerGap;
	bool showEnableBT;
	bool showRemoveBT;

	//Resize
	Direction resizeDirection;
	int resizerWidth;
	int resizerHeight;

	std::unique_ptr<ResizableCornerComponent> cornerResizer;
	std::unique_ptr<ResizableEdgeComponent> edgeResizer;
	ComponentBoundsConstrainer constrainer;

	Component * resizer;

	Label itemLabel;
	std::unique_ptr<BoolImageToggleUI> enabledBT;
	std::unique_ptr<ImageButton> removeBT;
	std::unique_ptr<WarningTargetUI> warningUI;

	//std::unique_ptr<Grabber> grabber;

	Array<Component *> contentComponents;

	void setContentSize(int contentWidth, int contentHeight);

	//minimode
	int getHeightWithoutContent();

	virtual void updateMiniModeUI();

	//void setGrabber(Grabber * newGrabber);

	virtual void resized() override;
	virtual void resizedInternalHeader(juce::Rectangle<int> &) {}
	virtual void resizedInternalContent(juce::Rectangle<int> &) {}
	virtual void resizedInternalFooter(juce::Rectangle<int> &) {}
	virtual void buttonClicked(Button *b) override;


	virtual void mouseDown(const MouseEvent &e) override;
	virtual void labelTextChanged(Label * l) override;
	virtual bool keyPressed(const KeyPress &e) override;

	virtual bool canStartDrag(const MouseEvent &e) override;

	virtual void containerChildAddressChangedAsync(ControllableContainer *) override;
	virtual void controllableFeedbackUpdateInternal(Controllable *) override;

	virtual void visibilityChanged() override;

	virtual void componentVisibilityChanged(Component &c) override;

	class ItemUIListener
	{
	public:
		virtual ~ItemUIListener() {}
		virtual void itemUIMiniModeChanged(BaseItemUI<T> *) {}
	};

	ListenerList<ItemUIListener> itemUIListeners;
	void addItemUIListener(ItemUIListener* newListener) { itemUIListeners.add(newListener); }
	void removeItemUIListener(ItemUIListener* listener) { itemUIListeners.remove(listener); }


private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseItemUI<T>)
};


template<class T>
BaseItemUI<T>::BaseItemUI(T * _item, Direction _resizeDirection) :
	BaseItemMinimalUI<T>(_item),
	margin(3),
	minContentHeight(2),
	headerHeight(16),
	headerGap(2),
	showEnableBT(true),
	showRemoveBT(true),
	resizeDirection(_resizeDirection),
	resizerWidth(0),
	resizerHeight(0),
	resizer(nullptr),
	itemLabel("itemLabel", dynamic_cast<BaseItem *>(this->inspectable.get())->niceName)
{

	itemLabel.setColour(itemLabel.backgroundColourId, Colours::transparentWhite);
	itemLabel.setColour(itemLabel.textColourId, TEXT_COLOR);

	itemLabel.setColour(itemLabel.backgroundWhenEditingColourId, Colours::black);
	itemLabel.setColour(itemLabel.textWhenEditingColourId, Colours::white);
	itemLabel.setColour(CaretComponent::caretColourId, Colours::orange);

	itemLabel.setFont((float)(headerHeight - 4));
	itemLabel.setJustificationType(Justification::centredLeft);

	itemLabel.setEditable(false, this->baseItem->nameCanBeChangedByUser);
	itemLabel.addListener(this);
	this->addAndMakeVisible(&itemLabel);

	/*
	dragAndDropEnabeld = dragDirection != NONE;
	if (dragDirection != NONE)
	{
		setGrabber(new Grabber(dragDirection == VERTICAL ? Grabber::VERTICAL : Grabber::HORIZONTAL));
	}
	*/

	switch (resizeDirection)
	{
	case NONE:
		break;

	case VERTICAL:
		resizerHeight = 8;
		constrainer.setMinimumHeight(headerHeight + headerGap + minContentHeight + resizerHeight);
		edgeResizer.reset(new ResizableEdgeComponent(this, &constrainer, ResizableEdgeComponent::bottomEdge));
		edgeResizer->setAlwaysOnTop(true);
		this->addAndMakeVisible(edgeResizer.get());
		break;

	case HORIZONTAL:
		resizerWidth = 4;
		constrainer.setMinimumWidth(20 + resizerWidth); // ??
		edgeResizer.reset(new ResizableEdgeComponent(this, &constrainer, ResizableEdgeComponent::rightEdge));
		edgeResizer->setAlwaysOnTop(true);
		this->addAndMakeVisible(edgeResizer.get());
		break;

	case ALL:
		resizerHeight = 10;
		constrainer.setMinimumSize(resizerWidth + 20, headerHeight + headerGap + minContentHeight + resizerHeight);
		cornerResizer.reset(new ResizableCornerComponent(this, &constrainer));
		cornerResizer->setAlwaysOnTop(true);
		this->addAndMakeVisible(cornerResizer.get());
		break;
	}

	if (this->baseItem->canBeDisabled)
	{
		enabledBT.reset(this->baseItem->enabled->createImageToggle(AssetManager::getInstance()->getPowerBT()));
		this->addAndMakeVisible(enabledBT.get());
	}

	if (this->baseItem->userCanRemove)
	{
		removeBT.reset(AssetManager::getInstance()->getRemoveBT());
		this->addAndMakeVisible(removeBT.get());
		removeBT->addListener(this);
	}

	if (this->baseItem->showWarningInUI)
	{
		warningUI.reset(new WarningTargetUI(this->baseItem));
		warningUI->addComponentListener(this);
		this->addChildComponent(warningUI.get());
	}

	this->setHighlightOnMouseOver(true);

	updateMiniModeUI();
}

template<class T>
BaseItemUI<T>::~BaseItemUI()
{
	if (removeBT != nullptr) removeBT->removeListener(this);
}

template<class T>
void BaseItemUI<T>::setContentSize(int contentWidth, int contentHeight)
{
	int targetHeight = getHeightWithoutContent() + contentHeight + this->getExtraHeight();
	int targetWidth = contentWidth + margin * 2 + resizerWidth + this->getExtraWidth();

	this->setSize(targetWidth, targetHeight);
}

template<class T>
int BaseItemUI<T>::getHeightWithoutContent()
{
	return headerHeight + margin * 2 + (this->baseItem->miniMode->boolValue() ? 0 : headerGap + resizerHeight);
}

template<class T>
void BaseItemUI<T>::updateMiniModeUI()
{
	//auto hide/show component in content section
	for (auto &c : contentComponents) c->setVisible(!this->baseItem->miniMode->boolValue());

	if (this->baseItem->miniMode->boolValue())
	{
		if (resizer != nullptr) this->removeChildComponent(resizer);

		int targetHeight = getHeightWithoutContent();
		this->setSize((int)this->baseItem->viewUISize->getPoint().x, targetHeight);
	}
	else
	{
		if (resizer != nullptr) this->addAndMakeVisible(resizer);

		int targetHeight = 0;
		int targetWidth = this->getWidth();

		switch (resizeDirection)
		{
		case ALL:
			targetWidth = (int)this->baseItem->viewUISize->getPoint().x;
			targetHeight = (int)this->baseItem->viewUISize->getPoint().y;
			break;
		case VERTICAL:
			targetHeight = (int)this->baseItem->listUISize->floatValue();
			break;
		case HORIZONTAL:
			targetWidth = (int)this->baseItem->listUISize->floatValue();
			break;
		case NONE:
			break;
		}

		setContentSize(targetWidth, targetHeight);
	}

	itemUIListeners.call(&ItemUIListener::itemUIMiniModeChanged, this);
}



template<class T>
void BaseItemUI<T>::resized()
{
	if (!this->isVisible()) return;

	//Header
	if (this->getWidth() == 0 || this->getHeight() == 0) return;
	juce::Rectangle<int> r = this->getMainBounds().reduced(margin);

	//Grabber

	juce::Rectangle<int> h = r.removeFromTop(headerHeight);


	if (enabledBT != nullptr && showEnableBT)
	{
		enabledBT->setBounds(h.removeFromLeft(h.getHeight()));
		h.removeFromLeft(2);
	}

	if (warningUI != nullptr && warningUI->isVisible())
	{
		warningUI->setBounds(h.removeFromLeft(h.getHeight())); //warning
		h.removeFromLeft(2);
	}

	if (removeBT != nullptr && showRemoveBT) removeBT->setBounds(h.removeFromRight(h.getHeight()));
	h.removeFromRight(2);

	resizedInternalHeader(h);
	itemLabel.setBounds(h.reduced(1));

	r.removeFromTop(headerGap);

	if (!this->baseItem->miniMode->boolValue())
	{
		if (resizeDirection == NONE)
		{
			int top = r.getY();
			resizedInternalContent(r);
			if (r.getWidth() == 0 || r.getHeight() == 0) return;
			setContentSize(r.getWidth(), r.getBottom() - top);
		}
		else
		{
			switch (resizeDirection)
			{
			case VERTICAL:
			{
				juce::Rectangle<int> fr = r.removeFromBottom(resizerHeight);
				resizedInternalFooter(fr);
				edgeResizer->setBounds(fr);
				this->baseItem->listUISize->setValue((float)r.getHeight());

			}
			break;

			case HORIZONTAL:
			{
				edgeResizer->setBounds(r.removeFromRight(resizerWidth));
				this->baseItem->listUISize->setValue((float)r.getWidth());
			}
			break;

			case ALL:
			{
				juce::Rectangle<int> fr = r.removeFromBottom(resizerHeight);
				resizedInternalFooter(fr);
				cornerResizer->setBounds(fr.withLeft(r.getWidth() - resizerHeight));
				this->baseItem->viewUISize->setPoint((float)r.getWidth() + this->getExtraWidth(), (float)r.getHeight() + this->getExtraHeight());
			}
			break;

			default:
				break;

			}
			resizedInternalContent(r);
		}
	}
}

template<class T>
void BaseItemUI<T>::buttonClicked(Button * b)
{
	if (b == removeBT.get())
	{
		if (this->baseItem->askConfirmationBeforeRemove && GlobalSettings::getInstance()->askBeforeRemovingItems->boolValue())
		{
			int result = AlertWindow::showOkCancelBox(AlertWindow::QuestionIcon, "Delete " + this->baseItem->niceName, "Are you sure you want to delete this ?", "Delete", "Cancel");
			if (result != 0)this->baseItem->remove();
		}
		else this->baseItem->remove();
	}
}


template<class T>
void BaseItemUI<T>::mouseDown(const MouseEvent & e)
{
	//if ((removeBT != nullptr && e.eventComponent == removeBT) || (enabledBT != nullptr && e.eventComponent == enabledBT->bt)) return;
	Button * b = dynamic_cast<Button *>(e.eventComponent);
	if (b != nullptr) return;

	TriggerUI * tui = dynamic_cast<TriggerUI *>(e.eventComponent);
	if (tui != nullptr) return;

	ParameterUI * pui = dynamic_cast<ParameterUI *>(e.eventComponent);
	if (pui != nullptr) return;

	BaseItemMinimalUI<T>::mouseDown(e);
}

template<class T>
void BaseItemUI<T>::labelTextChanged(Label * l)
{
	if (l == &itemLabel)
	{
		if (l->getText().isEmpty()) itemLabel.setText(this->baseItem->niceName, dontSendNotification); //avoid setting empty names
		else this->baseItem->setUndoableNiceName(l->getText());
	}
}

template<class T>
bool BaseItemUI<T>::keyPressed(const KeyPress & e)
{
	if (!this->inspectable.wasObjectDeleted() && this->item->isSelected)
	{
		if (e.getKeyCode() == KeyPress::F2Key)
		{
			this->itemLabel.showEditor();
			return true;
		}
	}

	return InspectableContentComponent::keyPressed(e);

}

template<class T>
bool BaseItemUI<T>::canStartDrag(const MouseEvent & e)
{
	return e.eventComponent == this || (e.eventComponent == &itemLabel && !itemLabel.isBeingEdited());
}


template<class T>
void BaseItemUI<T>::containerChildAddressChangedAsync(ControllableContainer *)
{

	itemLabel.setText(this->baseItem->niceName, dontSendNotification);
}

template<class T>
void BaseItemUI<T>::controllableFeedbackUpdateInternal(Controllable * c)
{
	BaseItemMinimalUI<T>::controllableFeedbackUpdateInternal(c);
	if (c == this->baseItem->miniMode) updateMiniModeUI();
}

template<class T>
void BaseItemUI<T>::visibilityChanged()
{
	resized();
}

template<class T>
void BaseItemUI<T>::componentVisibilityChanged(Component& c)
{
	if (&c == warningUI.get()) resized();
}
