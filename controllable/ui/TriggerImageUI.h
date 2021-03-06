/*
  ==============================================================================

    TriggerImageUI.h
    Created: 4 Jan 2017 1:32:47pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class TriggerImageUI :
	public TriggerUI,
	public Timer
{
public:
	TriggerImageUI(Trigger *, const Image &image);
	~TriggerImageUI();

	Image onImage;
	Image offImage;

	bool drawTriggering;

	void paint(Graphics &g) override;
	void triggerTriggered(const Trigger * p) override;

	void mouseDownInternal(const MouseEvent &e) override;


	// Inherited via Timer
	virtual void timerCallback() override;
};

