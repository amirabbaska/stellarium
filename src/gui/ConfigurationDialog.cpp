/*
 *
 * Stellarium
 * Copyright (C) 2008 Fabien Chereau
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "Dialog.hpp"
#include "ConfigurationDialog.hpp"
#include "StelMainGraphicsView.hpp"
#include "StelMainWindow.hpp"
#include "ui_configurationDialog.h"
#include "StelApp.hpp"
#include "StelFileMgr.hpp"
#include "StelCore.hpp"
#include "StelLocaleMgr.hpp"
#include "Projector.hpp"
#include "Navigator.hpp"
#include "StelCore.hpp"
#include "MovementMgr.hpp"
#include "StelModuleMgr.hpp"
#include "SkyDrawer.hpp"
#include "StelGui.hpp"
#include "StelGuiItems.hpp"
#include "Location.hpp"
#include "LandscapeMgr.hpp"
#include "StelSkyCultureMgr.hpp"
#include "SolarSystem.hpp"
#include "MeteorMgr.hpp"
#include "ConstellationMgr.hpp"
#include "StarMgr.hpp"
#include "NebulaMgr.hpp"
#include "GridLinesMgr.hpp"

#include <QSettings>
#include <QDebug>
#include <QFrame>
#include <QFile>
#include <QFileDialog>

#include "StelAppGraphicsScene.hpp"

ConfigurationDialog::ConfigurationDialog()
{
	ui = new Ui_configurationDialogForm;
}

ConfigurationDialog::~ConfigurationDialog()
{
	delete ui;
}

void ConfigurationDialog::languageChanged()
{
	if (dialog)
		ui->retranslateUi(dialog);
}

void ConfigurationDialog::styleChanged()
{
	// Nothing for now
}

void ConfigurationDialog::createDialogContent()
{
	Projector* proj = StelApp::getInstance().getCore()->getProjection();
	Navigator* nav = StelApp::getInstance().getCore()->getNavigation();
	MovementMgr* mvmgr = (MovementMgr*)GETSTELMODULE("MovementMgr");
	StelGui* gui = (StelGui*)GETSTELMODULE("StelGui");
	
	ui->setupUi(dialog);
	
	// Set the main tab activated by default
	ui->configurationTabWidget->setCurrentIndex(0);
	
	connect(ui->closeStelWindow, SIGNAL(clicked()), this, SLOT(close()));
	
	// Main tab
	// Fill the language list widget from the available list
// 	QListWidget* c = ui->programLanguageListWidget;
// 	c->clear();
// 	c->addItems(Translator::globalTranslator.getAvailableLanguagesNamesNative(StelApp::getInstance().getFileMgr().getLocaleDir()));
// 	
// 	QString l = Translator::iso639_1CodeToNativeName(appLang);
// 	QList<QListWidgetItem*> litem = c->findItems(l, Qt::MatchExactly);
// 	if (litem.empty() && appLang.contains('_'))
// 	{
// 		l = appLang.left(appLang.indexOf('_'));
// 		l=Translator::iso639_1CodeToNativeName(l);
// 		litem = c->findItems(l, Qt::MatchExactly);
// 	}
// 	if (!litem.empty())
// 		c->setCurrentItem(litem.at(0));
// 	connect(c, SIGNAL(currentTextChanged(const QString&)), this, SLOT(languageChanged(const QString&)));

	QString appLang = StelApp::getInstance().getLocaleMgr().getAppLanguage();
	QComboBox* cb = ui->programLanguageComboBox;
	cb->clear();
	cb->addItems(Translator::globalTranslator.getAvailableLanguagesNamesNative(StelApp::getInstance().getFileMgr().getLocaleDir()));
	QString l2 = Translator::iso639_1CodeToNativeName(appLang);
	int lt = cb->findText(l2, Qt::MatchExactly);
	if (lt == -1 && appLang.contains('_'))
	{
		l2 = appLang.left(appLang.indexOf('_'));
		l2=Translator::iso639_1CodeToNativeName(l2);
		lt = cb->findText(l2, Qt::MatchExactly);
	}
	if (lt!=-1)
		cb->setCurrentIndex(lt);
	connect(cb, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(languageChanged(const QString&)));
	
	// Selected object info
	if (gui->getInfoPanel()->getInfoTextFilters() == (StelObject::InfoStringGroup)0)
		ui->noSelectedInfoRadio->setChecked(true);
	else if (gui->getInfoPanel()->getInfoTextFilters() == StelObject::InfoStringGroup(StelObject::ShortInfo))
		ui->briefSelectedInfoRadio->setChecked(true);
	else
		ui->allSelectedInfoRadio->setChecked(true);
	connect(ui->noSelectedInfoRadio, SIGNAL(released()), this, SLOT(setNoSelectedInfo()));
	connect(ui->allSelectedInfoRadio, SIGNAL(released()), this, SLOT(setAllSelectedInfo()));
	connect(ui->briefSelectedInfoRadio, SIGNAL(released()), this, SLOT(setBriefSelectedInfo()));
	
	// Navigation tab
	// Startup time
	if (nav->getStartupTimeMode()=="actual")
		ui->systemTimeRadio->setChecked(true);
	else if (nav->getStartupTimeMode()=="today")
		ui->todayRadio->setChecked(true);
	else
		ui->fixedTimeRadio->setChecked(true);
	connect(ui->systemTimeRadio, SIGNAL(clicked(bool)), this, SLOT(setStartupTimeMode()));
	connect(ui->todayRadio, SIGNAL(clicked(bool)), this, SLOT(setStartupTimeMode()));
	connect(ui->fixedTimeRadio, SIGNAL(clicked(bool)), this, SLOT(setStartupTimeMode()));
	
	ui->todayTimeSpinBox->setTime(nav->getInitTodayTime());
	connect(ui->todayTimeSpinBox, SIGNAL(timeChanged(QTime)), nav, SLOT(setInitTodayTime(QTime)));
	ui->fixedDateTimeEdit->setDateTime(StelUtils::jdToQDateTime(nav->getPresetSkyTime()));
	connect(ui->fixedDateTimeEdit, SIGNAL(dateTimeChanged(QDateTime)), nav, SLOT(setPresetSkyTime(QDateTime)));

	ui->enableKeysNavigationCheckBox->setChecked(mvmgr->getFlagEnableMoveKeys() || mvmgr->getFlagEnableZoomKeys());
	ui->enableMouseNavigationCheckBox->setChecked(mvmgr->getFlagEnableMouseNavigation());
	connect(ui->enableKeysNavigationCheckBox, SIGNAL(toggled(bool)), mvmgr, SLOT(setFlagEnableMoveKeys(bool)));
	connect(ui->enableKeysNavigationCheckBox, SIGNAL(toggled(bool)), mvmgr, SLOT(setFlagEnableZoomKeys(bool)));
	connect(ui->enableMouseNavigationCheckBox, SIGNAL(toggled(bool)), mvmgr, SLOT(setFlagEnableMouseNavigation(bool)));
	
	// Tools tab
	ui->sphericMirrorCheckbox->setChecked(StelAppGraphicsScene::getInstance().getViewPortDistorterType() == "fisheye_to_spheric_mirror");
	connect(ui->sphericMirrorCheckbox, SIGNAL(toggled(bool)), this, SLOT(setSphericMirror(bool)));
	ui->gravityLabelCheckbox->setChecked(proj->getFlagGravityLabels());
	connect(ui->gravityLabelCheckbox, SIGNAL(toggled(bool)), proj, SLOT(setFlagGravityLabels(bool)));
	ui->discViewportCheckbox->setChecked(proj->getMaskType() == Projector::Disk);
	connect(ui->discViewportCheckbox, SIGNAL(toggled(bool)), this, SLOT(setDiskViewport(bool)));
	ui->autoZoomResetsDirectionCheckbox->setChecked(mvmgr->getFlagAutoZoomOutResetsDirection());
	connect(ui->autoZoomResetsDirectionCheckbox, SIGNAL(toggled(bool)), mvmgr, SLOT(setFlagAutoZoomOutResetsDirection(bool)));
	
	ui->showFlipButtonsCheckbox->setChecked(gui->getFlagShowFlipButtons());
	connect(ui->showFlipButtonsCheckbox, SIGNAL(toggled(bool)), gui, SLOT(setFlagShowFlipButtons(bool)));
	
	ui->mouseTimeoutCheckbox->setChecked(StelAppGraphicsScene::getInstance().getFlagCursorTimeout());
	ui->mouseTimeoutSpinBox->setValue(StelAppGraphicsScene::getInstance().getCursorTimeout());
	connect(ui->mouseTimeoutCheckbox, SIGNAL(clicked()), this, SLOT(cursorTimeOutChanged()));
	connect(ui->mouseTimeoutCheckbox, SIGNAL(toggled(bool)), this, SLOT(cursorTimeOutChanged()));
	connect(ui->mouseTimeoutSpinBox, SIGNAL(valueChanged(double)), this, SLOT(cursorTimeOutChanged(double)));
	
	connect(ui->setViewingOptionAsDefaultPushButton, SIGNAL(clicked()), this, SLOT(saveCurrentViewOptions()));
	connect(ui->restoreDefaultsButton, SIGNAL(clicked()), this, SLOT(setDefaultViewOptions()));

	ui->screenshotDirEdit->setText(StelApp::getInstance().getFileMgr().getScreenshotDir());
	connect(ui->screenshotDirEdit, SIGNAL(textChanged(QString)), this, SLOT(selectScreenshotDir(QString)));
	connect(ui->screenshotBrowseButton, SIGNAL(clicked()), this, SLOT(browseForScreenshotDir()));
	
	ui->invertScreenShotColorsCheckBox->setChecked(StelMainGraphicsView::getInstance().getFlagInvertScreenShotColors());
	connect(ui->invertScreenShotColorsCheckBox, SIGNAL(toggled(bool)), &StelMainGraphicsView::getInstance(), SLOT(setFlagInvertScreenShotColors(bool)));
	
	updateConfigLabels();
}

void ConfigurationDialog::languageChanged(const QString& langName)
{
	StelApp::getInstance().getLocaleMgr().setAppLanguage(Translator::nativeNameToIso639_1Code(langName));
	StelApp::getInstance().getLocaleMgr().setSkyLanguage(Translator::nativeNameToIso639_1Code(langName));
}

void ConfigurationDialog::setStartupTimeMode(void)
{
	Navigator* nav = StelApp::getInstance().getCore()->getNavigation();
	Q_ASSERT(nav);

	if (ui->systemTimeRadio->isChecked())
		StelApp::getInstance().getCore()->getNavigation()->setStartupTimeMode("actual");
	else if (ui->todayRadio->isChecked())
		StelApp::getInstance().getCore()->getNavigation()->setStartupTimeMode("today");
	else
		StelApp::getInstance().getCore()->getNavigation()->setStartupTimeMode("preset");

	nav->setInitTodayTime(ui->todayTimeSpinBox->time());
	nav->setPresetSkyTime(ui->fixedDateTimeEdit->dateTime());
}

void ConfigurationDialog::setDiskViewport(bool b)
{
	Projector* proj = StelApp::getInstance().getCore()->getProjection();
	assert(proj);
	if (b)
		proj->setMaskType(Projector::Disk);
	else
		proj->setMaskType(Projector::None);
}

void ConfigurationDialog::setSphericMirror(bool b)
{
	if (b)
		StelAppGraphicsScene::getInstance().setViewPortDistorterType("fisheye_to_spheric_mirror");
	else
		StelAppGraphicsScene::getInstance().setViewPortDistorterType("none");
}

void ConfigurationDialog::setNoSelectedInfo(void)
{
	StelGui* newGui = (StelGui*)GETSTELMODULE("StelGui");
	assert(newGui);
	newGui->getInfoPanel()->setInfoTextFilters(StelObject::InfoStringGroup(0));
}

void ConfigurationDialog::setAllSelectedInfo(void)
{
	StelGui* newGui = (StelGui*)GETSTELMODULE("StelGui");
	assert(newGui);
	newGui->getInfoPanel()->setInfoTextFilters(StelObject::InfoStringGroup(StelObject::AllInfo));
}

void ConfigurationDialog::setBriefSelectedInfo(void)
{
	StelGui* newGui = (StelGui*)GETSTELMODULE("StelGui");
	assert(newGui);
	newGui->getInfoPanel()->setInfoTextFilters(StelObject::InfoStringGroup(StelObject::ShortInfo));
}

void ConfigurationDialog::cursorTimeOutChanged()
{
	StelAppGraphicsScene::getInstance().setFlagCursorTimeout(ui->mouseTimeoutCheckbox->isChecked());
	StelAppGraphicsScene::getInstance().setCursorTimeout(ui->mouseTimeoutSpinBox->value());
}

void ConfigurationDialog::browseForScreenshotDir()
{
	QString oldScreenshorDir = StelApp::getInstance().getFileMgr().getScreenshotDir();
	QString newScreenshotDir = QFileDialog::getExistingDirectory(NULL, q_("Select screenshot directory"), oldScreenshorDir, QFileDialog::ShowDirsOnly);

	if (!newScreenshotDir.isEmpty()) {
		// remove trailing slash 
		if (newScreenshotDir.right(1) == "/")
			newScreenshotDir = newScreenshotDir.left(newScreenshotDir.length()-1);

		ui->screenshotDirEdit->setText(newScreenshotDir);
	}
}

void ConfigurationDialog::selectScreenshotDir(const QString& dir)
{
	try
	{
		StelApp::getInstance().getFileMgr().setScreenshotDir(dir);
	}
	catch (std::runtime_error& e)
	{
		// nop
		// this will happen when people are only half way through typing dirs
	}
}

// Save the current viewing option including landscape, location and sky culture
// This doesn't include the current viewing direction, time and FOV since those have specific controls
void ConfigurationDialog::saveCurrentViewOptions()
{
	QSettings* conf = StelApp::getInstance().getSettings();
	Q_ASSERT(conf);

	LandscapeMgr* lmgr = (LandscapeMgr*)GETSTELMODULE("LandscapeMgr");
	Q_ASSERT(lmgr);
	SolarSystem* ssmgr = (SolarSystem*)GETSTELMODULE("SolarSystem");
	Q_ASSERT(ssmgr);
	MeteorMgr* mmgr = (MeteorMgr*)GETSTELMODULE("MeteorMgr");
	Q_ASSERT(mmgr);
	SkyDrawer* skyd = StelApp::getInstance().getCore()->getSkyDrawer();
	Q_ASSERT(skyd);
	ConstellationMgr* cmgr = (ConstellationMgr*)GETSTELMODULE("ConstellationMgr");
	Q_ASSERT(cmgr);
	StarMgr* smgr = (StarMgr*)GETSTELMODULE("StarMgr");
	Q_ASSERT(smgr);
	NebulaMgr* nmgr = (NebulaMgr*)GETSTELMODULE("NebulaMgr");
	Q_ASSERT(nmgr);
	GridLinesMgr* glmgr = (GridLinesMgr*)GETSTELMODULE("GridLinesMgr");
	Q_ASSERT(glmgr);
	StelGui* gui = (StelGui*)GETSTELMODULE("StelGui");
	Q_ASSERT(gui);
	MovementMgr* mvmgr = (MovementMgr*)GETSTELMODULE("MovementMgr");
	Q_ASSERT(mvmgr);
	Navigator* nav = StelApp::getInstance().getCore()->getNavigation();
	Q_ASSERT(nav);
	Projector* proj = StelApp::getInstance().getCore()->getProjection();
	Q_ASSERT(proj);

	// view dialog / sky tab settings
	conf->setValue("stars/absolute_scale", skyd->getAbsoluteStarScale());
	conf->setValue("stars/relative_scale", skyd->getRelativeStarScale());
	conf->setValue("stars/flag_star_twinkle", skyd->getFlagTwinkle());
	conf->setValue("stars/star_twinkle_amount", skyd->getTwinkleAmount());
	conf->setValue("viewing/use_luminance_adaptation", skyd->getFlagLuminanceAdaptation());
	conf->setValue("astro/flag_planets", ssmgr->getFlagPlanets());
	conf->setValue("astro/flag_planets_hints", ssmgr->getFlagHints());
	conf->setValue("astro/flag_planets_orbits", ssmgr->getFlagOrbits());
	conf->setValue("astro/flag_light_travel_time", ssmgr->getFlagLightTravelTime());
	conf->setValue("viewing/flag_moon_scaled", ssmgr->getFlagMoonScale());
	conf->setValue("astro/meteor_rate", mmgr->getZHR());

	// view dialog / markings tab settings
	conf->setValue("viewing/flag_azimutal_grid", glmgr->getFlagAzimutalGrid());
	conf->setValue("viewing/flag_equatorial_grid", glmgr->getFlagEquatorGrid());
	conf->setValue("viewing/flag_equator_line", glmgr->getFlagEquatorLine());
	conf->setValue("viewing/flag_ecliptic_line", glmgr->getFlagEclipticLine());
	conf->setValue("viewing/flag_meridian_line", glmgr->getFlagMeridianLine());
	conf->setValue("viewing/flag_equatorial_J2000_grid", glmgr->getFlagEquatorJ2000Grid());
	conf->setValue("viewing/flag_cardinal_points", lmgr->getFlagCardinalsPoints());
	conf->setValue("viewing/flag_constellation_drawing", cmgr->getFlagLines());
	conf->setValue("viewing/flag_constellation_name", cmgr->getFlagLabels());
	conf->setValue("viewing/flag_constellation_boundaries", cmgr->getFlagBoundaries());
	conf->setValue("viewing/flag_constellation_art", cmgr->getFlagArt());
	conf->setValue("viewing/constellation_art_intensity", cmgr->getArtIntensity());
	conf->setValue("astro/flag_star_name", smgr->getFlagLabels());
	conf->setValue("stars/labels_amount", smgr->getLabelsAmount());
	conf->setValue("astro/flag_planets_labels", ssmgr->getFlagLabels());
	conf->setValue("astro/labels_amount", ssmgr->getLabelsAmount());
	conf->setValue("astro/nebula_hints_amount", nmgr->getHintsAmount());
	conf->setValue("astro/flag_nebula_name", nmgr->getFlagHints());
	conf->setValue("projection/type", StelApp::getInstance().getCore()->getProjection()->getCurrentMapping().getId());

	// view dialog / landscape tab settings
	lmgr->setDefaultLandscapeID(lmgr->getCurrentLandscapeID());
	conf->setValue("landscape/flag_landscape_sets_location", lmgr->getFlagLandscapeSetsLocation());
	conf->setValue("landscape/flag_landscape", lmgr->getFlagLandscape());
	conf->setValue("landscape/flag_atmosphere", lmgr->getFlagAtmosphere());
	conf->setValue("landscape/flag_fog", lmgr->getFlagFog());
	conf->setValue("stars/init_bortle_scale", StelApp::getInstance().getCore()->getSkyDrawer()->getBortleScale());

	// view dialog / starlore tab
	StelApp::getInstance().getSkyCultureMgr().setDefaultSkyCultureID(StelApp::getInstance().getSkyCultureMgr().getCurrentSkyCultureID());
	
	// Save default location
	StelApp::getInstance().getCore()->getNavigation()->setDefaultLocationID(StelApp::getInstance().getCore()->getNavigation()->getCurrentLocation().getID());

	// configuration dialog / main tab
	QString langName = StelApp::getInstance().getLocaleMgr().getAppLanguage();
	conf->setValue("localization/app_locale", Translator::nativeNameToIso639_1Code(langName));

	if (gui->getInfoPanel()->getInfoTextFilters() == (StelObject::InfoStringGroup)0)
		conf->setValue("gui/selected_object_info", "none");
	else if (gui->getInfoPanel()->getInfoTextFilters() == StelObject::InfoStringGroup(StelObject::ShortInfo))
		conf->setValue("gui/selected_object_info", "short");
	else
		conf->setValue("gui/selected_object_info", "all");

	// configuration dialog / navigation tab
	conf->setValue("navigation/flag_enable_zoom_keys", mvmgr->getFlagEnableZoomKeys());
	conf->setValue("navigation/flag_enable_mouse_navigation", mvmgr->getFlagEnableMouseNavigation());
	conf->setValue("navigation/flag_enable_move_keys", mvmgr->getFlagEnableMoveKeys());
	conf->setValue("navigation/startup_time_mode", nav->getStartupTimeMode());
	conf->setValue("navigation/today_time", nav->getInitTodayTime());
	conf->setValue("navigation/preset_sky_time", nav->getPresetSkyTime());
	conf->setValue("navigation/init_fov", proj->getInitFov());

	// configuration dialog / tools tab
	conf->setValue("gui/flag_show_flip_buttons", gui->getFlagShowFlipButtons());
	conf->setValue("video/distorter", StelAppGraphicsScene::getInstance().getViewPortDistorterType());
	conf->setValue("projection/viewport", Projector::maskTypeToString(proj->getMaskType()));
	conf->setValue("viewing/flag_gravity_labels", proj->getFlagGravityLabels());
	conf->setValue("navigation/auto_zoom_out_resets_direction", mvmgr->getFlagAutoZoomOutResetsDirection());
	conf->setValue("gui/flag_mouse_cursor_timeout", StelAppGraphicsScene::getInstance().getFlagCursorTimeout());
	conf->setValue("gui/mouse_cursor_timeout", StelAppGraphicsScene::getInstance().getCursorTimeout());
	
	StelApp::getInstance().getCore()->getProjection()->setInitFov(StelApp::getInstance().getCore()->getProjection()->getFov());
	StelApp::getInstance().getCore()->getNavigation()->setInitViewDirectionToCurrent();
	conf->setValue("main/screenshot_dir", StelApp::getInstance().getFileMgr().getScreenshotDir());

	// full screen and window size
	conf->setValue("video/fullscreen", StelMainWindow::getInstance().getFullScreen());
	if (!StelMainWindow::getInstance().getFullScreen())
	{
		conf->setValue("video/screen_w", StelMainWindow::getInstance().size().width());
		conf->setValue("video/screen_h", StelMainWindow::getInstance().size().height());
	}

	// clear the restore defaults flag if it is set.
	conf->setValue("main/restore_defaults", false);
	
	updateConfigLabels();
}

void ConfigurationDialog::updateConfigLabels()
{
	ui->startupFOVLabel->setText(q_("Startup FOV: %1%2").arg(StelApp::getInstance().getCore()->getProjection()->getFov()).arg(QChar(0x00B0)));
	
	double az, alt;
	const Vec3d v = StelApp::getInstance().getCore()->getNavigation()->getInitViewingDirection();
	StelUtils::rectToSphe(&az, &alt, v);
	az = 3.*M_PI - az;  // N is zero, E is 90 degrees
	if (az > M_PI*2)
		az -= M_PI*2;    
	ui->startupDirectionOfViewlabel->setText(q_("Startup direction of view Az/Alt: %1/%2").arg(StelUtils::radToDmsStr(az), StelUtils::radToDmsStr(alt)));
}

void ConfigurationDialog::setDefaultViewOptions()
{
	QSettings* conf = StelApp::getInstance().getSettings();
	Q_ASSERT(conf);

	conf->setValue("main/restore_defaults", true);
}

