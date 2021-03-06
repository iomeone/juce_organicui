/*
  ==============================================================================

    DashboardItemManagerUI.h
    Created: 23 Apr 2017 12:33:58pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class DashboardItemManagerUI :
	public BaseManagerViewUI<DashboardItemManager, DashboardItem, BaseItemMinimalUI<DashboardItem>>
{
public:
	DashboardItemManagerUI(DashboardItemManager * manager);
	~DashboardItemManagerUI();


	bool isInterestedInDragSource(const SourceDetails & dragSourceDetails) override;
	void itemDropped(const SourceDetails &details) override;

	BaseItemMinimalUI<DashboardItem> * createUIForItem(DashboardItem *) override;
};