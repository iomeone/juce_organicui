/*
  ==============================================================================

    ScriptUtil.h
    Created: 21 Feb 2017 8:44:28am
    Author:  Ben

  ==============================================================================
*/

#pragma once

class ScriptUtil :
	public ScriptTarget
{
public:
	juce_DeclareSingleton(ScriptUtil, true)

	ScriptUtil();
	~ScriptUtil() {}

	static var getTime(const var::NativeFunctionArgs &a);
	static var getFloatFromBytes(const var::NativeFunctionArgs &a);
	static var getInt32FromBytes(const var::NativeFunctionArgs &a);
	static var getInt64FromBytes(const var::NativeFunctionArgs &a);
	
	static var cosFromScript(const var::NativeFunctionArgs &a);
	static var sinFromScript(const var::NativeFunctionArgs &a);
	static var atan2FromScript(const var::NativeFunctionArgs &a);

	static var toDegrees(const var::NativeFunctionArgs &a);
	static var toRadians(const var::NativeFunctionArgs &a);
};
