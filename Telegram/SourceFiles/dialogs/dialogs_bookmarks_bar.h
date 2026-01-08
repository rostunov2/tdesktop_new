/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "ui/rp_widget.h"

namespace Window {
class SessionController;
} // namespace Window

namespace Ui {
class FixedHeightWidget;
class VerticalLayout;
} // namespace Ui

namespace Dialogs {

class BookmarksBar final : public Ui::RpWidget {
public:
	BookmarksBar(
		QWidget *parent,
		not_null<Window::SessionController*> controller);

private:
	void setupHeader();
	void rebuildRows();
	void showAddBookmarkBox();

	not_null<Window::SessionController*> _controller;
	base::unique_qptr<Ui::VerticalLayout> _layout;
	Ui::FixedHeightWidget *_header = nullptr;
	std::vector<Ui::RpWidget*> _rows;
};

} // namespace Dialogs
