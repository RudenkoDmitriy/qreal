#pragma once

#include <qrgui/plugins/pluginManager/toolPluginManager.h>

#include "qrgui/brandManager/brandManagerDeclSpec.h"
#include "qrgui/brandManager/fonts.h"
#include "qrgui/brandManager/styles.h"

namespace qReal {

/// Provides information about everything about application appearance: fonts, styles and so on
class QRGUI_BRAND_MANAGER_EXPORT BrandManager
{
public:
	static BrandManager &instance();

	/// Retrieves from plugins customization information
	static void configure(const ToolPluginManager *toolPluginManager);

	// TODO: Pluginize two following methods

	/// Returns information about all fonts in system
	static const Fonts *fonts();

	/// Returns information about all json stylesheets in system
	static const Styles *styles();

	/// Returns application`s name and probably version
	static QString applicationName();

	/// Returns application`s logo image in wide format
	static const QImage applicationLogo();

	/// Returns small application`s icon
	static const QIcon applicationIcon();

private:
	BrandManager();
	~BrandManager();

	Customizer *mCustomizer;  // Doesn`t take ownership
	const Fonts *mFonts;  // Takes ownership
	const Styles *mStyles;  // Takes ownership
};

}
