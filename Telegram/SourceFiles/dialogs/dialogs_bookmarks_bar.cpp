/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "dialogs/dialogs_bookmarks_bar.h"

#include "core/application.h"
#include "core/click_handler_types.h"
#include "core/core_settings.h"
#include "lang/lang_hardcoded.h"
#include "lang/lang_keys.h"
#include "ui/layers/layer_widget.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/fields/input_field.h"
#include "ui/widgets/labels.h"
#include "styles/style_dialogs.h"
#include "styles/style_layers.h"
#include "styles/style_widgets.h"

#include <algorithm>

namespace Dialogs {

BookmarksBar::BookmarksBar(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: RpWidget(parent)
, _controller(controller)
, _layout(base::make_unique_q<Ui::VerticalLayout>(this)) {
	setupHeader();
	rebuildRows();

	widthValue() | rpl::on_next([=](int width) {
		_layout->resizeToWidth(width);
	}, lifetime());

	_layout->heightValue() | rpl::on_next([=](int height) {
		resize(width(), height);
	}, lifetime());

	Core::App().settings().bookmarksChanges(
	) | rpl::on_next([=] {
		rebuildRows();
	}, lifetime());
}

void BookmarksBar::setupHeader() {
	const auto header = _layout->add(object_ptr<Ui::FixedHeightWidget>(
		_layout.get(),
		st::searchedBarHeight));
	_header = header;
	const auto label = Ui::CreateChild<Ui::FlatLabel>(
		header,
		rpl::single(Lang::Hard::BookmarksTitle()),
		st::searchedBarLabel);
	const auto add = Ui::CreateChild<Ui::LinkButton>(
		header,
		Lang::Hard::BookmarksAdd(),
		st::searchedBarLink);
	add->setClickedCallback([=] {
		showAddBookmarkBox();
	});

	rpl::combine(
		header->sizeValue(),
		add->widthValue()
	) | rpl::on_next([=](QSize size, int addWidth) {
		const auto x = st::searchedBarPosition.x();
		const auto y = st::searchedBarPosition.y();
		add->moveToRight(x, y, size.width());
		label->resizeToWidth(size.width() - x - addWidth - x);
		label->moveToLeft(x, y, size.width());
	}, header->lifetime());

	header->paintRequest() | rpl::on_next([=](QRect clip) {
		QPainter(header).fillRect(clip, st::searchedBarBg);
	}, header->lifetime());
}

void BookmarksBar::rebuildRows() {
	for (const auto row : _rows) {
		delete row;
	}
	_rows.clear();

	const auto &bookmarks = Core::App().settings().bookmarks();
	for (const auto &link : bookmarks) {
		const auto row = _layout->add(object_ptr<Ui::FixedHeightWidget>(
			_layout.get(),
			st::searchedBarHeight));
		const auto raw = row.data();
		const auto linkButton = Ui::CreateChild<Ui::LinkButton>(
			raw,
			link,
			st::defaultLinkButton);
		const auto removeButton = Ui::CreateChild<Ui::LinkButton>(
			raw,
			Lang::Hard::BookmarksRemove(),
			st::searchedBarLink);
		linkButton->setClickedCallback([=] {
			UrlClickHandler::Open(link);
		});
		removeButton->setClickedCallback([=] {
			Core::App().settings().removeBookmark(link);
		});
		rpl::combine(
			raw->sizeValue(),
			removeButton->widthValue()
		) | rpl::on_next([=](QSize size, int removeWidth) {
			const auto x = st::searchedBarPosition.x();
			const auto y = st::searchedBarPosition.y();
			const auto right = x;
			const auto available = std::max(
				0,
				size.width() - x - right - removeWidth);
			linkButton->resizeToWidth(available);
			linkButton->moveToLeft(x, y, size.width());
			removeButton->moveToRight(right, y, size.width());
		}, raw->lifetime());
		_rows.push_back(raw);
	}
}

void BookmarksBar::showAddBookmarkBox() {
	_controller->show(Box([=](not_null<Ui::GenericBox*> box) {
		box->setTitle(Lang::Hard::BookmarksAddTitle());

		const auto field = box->addRow(object_ptr<Ui::InputField>(
			box,
			st::defaultInputField,
			Ui::InputField::Mode::NoNewlines,
			Lang::Hard::BookmarksInputPlaceholder()));

		const auto submit = [=] {
			const auto link = field->getLastText().trimmed();
			if (link.isEmpty()) {
				field->showError();
				return;
			}
			Core::App().settings().addBookmark(link);
			box->closeBox();
		};

		field->submits(
		) | rpl::on_next([=](Qt::KeyboardModifiers modifiers) {
			if (!(modifiers & Qt::ShiftModifier)) {
				submit();
			}
		}, box->lifetime());

		box->addButton(Lang::Hard::BookmarksAddAction(), submit);
		box->addButton(tr::lng_cancel(), [=] {
			box->closeBox();
		});
		box->setFocusCallback([=] {
			field->setFocusFast();
		});
	}));
}

} // namespace Dialogs
