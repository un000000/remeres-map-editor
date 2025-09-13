//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "find_item_window.h"
#include "common_windows.h"
#include "gui.h"
#include "items.h"
#include "brush.h"
#include "raw_brush.h"
#include "item_filter.h"

BEGIN_EVENT_TABLE(FindItemDialog, wxDialog)
EVT_TIMER(wxID_ANY, FindItemDialog::OnInputTimer)
EVT_BUTTON(wxID_OK, FindItemDialog::OnClickOK)
EVT_BUTTON(wxID_CANCEL, FindItemDialog::OnClickCancel)
END_EVENT_TABLE()

void FindItemDialog::CreatePropertyRadioBoxes(wxStaticBoxSizer* propertiesBoxSizer) {
	wxArrayString choices;
	choices.Add("Any");
	choices.Add("Yes");
	choices.Add("No");

	auto createPropertyRow = [&](const wxString &label, wxRadioBox*&radioBox, int defaultSelection = 0, bool enabled = true) {
		wxBoxSizer* rowSizer = new wxBoxSizer(wxHORIZONTAL);

		wxStaticText* propLabel = new wxStaticText(propertiesBoxSizer->GetStaticBox(), wxID_ANY, label);
		rowSizer->Add(propLabel, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);

		radioBox = new wxRadioBox(propertiesBoxSizer->GetStaticBox(), wxID_ANY, "", wxDefaultPosition, wxDefaultSize, choices, 3, wxRA_HORIZONTAL);
		radioBox->SetSelection(defaultSelection);
		radioBox->Enable(enabled);
		rowSizer->Add(radioBox, 0, wxRIGHT, 2);

		propertiesBoxSizer->Add(rowSizer, 0, wxEXPAND | wxTOP, 1);
	};

	createPropertyRow("Unpassable", unpassableRadio);
	createPropertyRow("Movable", movableRadio);
	createPropertyRow("Block Missiles", blockMissilesRadio);
	createPropertyRow("Block Pathfinder", blockPathfinderRadio);
	createPropertyRow("Readable", readableRadio);
	createPropertyRow("Writeable", writeableRadio);
	createPropertyRow("Pickupable", pickupableRadio, onlyPickupables ? 1 : 0, !onlyPickupables);
	createPropertyRow("Stackable", stackableRadio);
	createPropertyRow("Rotatable", rotatableRadio);
	createPropertyRow("Hangable", hangableRadio);
	createPropertyRow("Hook East", hookEastRadio);
	createPropertyRow("Hook South", hookSouthRadio);
	createPropertyRow("Has Elevation", hasElevationRadio);
	createPropertyRow("Ignore Look", ignoreLookRadio);
	createPropertyRow("Floor Change", floorChangeRadio);
	createPropertyRow("Always on bottom", alwaysOnBottomRadio);
	createPropertyRow("Is ground", isGroundRadio);
}

FindItemDialog::FindItemDialog(wxWindow* parent, const wxString &title, bool onlyPickupables /* = false*/, bool onSelection /* = false*/) :
	wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(800, 900), wxDEFAULT_DIALOG_STYLE),
	inputTimer(this),
	onlyPickupables(onlyPickupables),
	onSelection(onSelection) {
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* boxSizer = newd wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer* optionsBoxSizer = newd wxBoxSizer(wxVERTICAL);

	wxString radioBoxChoices[] = { "Find by Item ID",
								   "Find by Name",
								   "Find by Types",
								   "Find by Tile Types",
								   "Find by Properties" };

	int radioBoxChoicesSize = sizeof(radioBoxChoices) / sizeof(wxString);
	optionsRadioBox = newd wxRadioBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, radioBoxChoicesSize, radioBoxChoices, 1, wxRA_SPECIFY_COLS);
	optionsRadioBox->SetSelection(SearchMode::ItemIDs);
	optionsBoxSizer->Add(optionsRadioBox, 0, wxALL | wxEXPAND, 5);

	wxStaticBoxSizer* itemIdBoxSizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Item ID"), wxVERTICAL);
	itemIdSpin = newd wxSpinCtrl(itemIdBoxSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 100, g_items.getMaxID(), 100);
	itemIdBoxSizer->Add(itemIdSpin, 0, wxALL | wxEXPAND, 5);
	optionsBoxSizer->Add(itemIdBoxSizer, 1, wxALL | wxEXPAND, 5);

	wxStaticBoxSizer* nameBoxSizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Name"), wxVERTICAL);
	nameTextInput = newd wxTextCtrl(nameBoxSizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	nameTextInput->Enable(false);
	nameBoxSizer->Add(nameTextInput, 0, wxALL | wxEXPAND, 5);
	optionsBoxSizer->Add(nameBoxSizer, 1, wxALL | wxEXPAND, 5);

	// spacer
	optionsBoxSizer->Add(0, 0, 4, wxALL | wxEXPAND, 5);

	buttonsBoxSizer = newd wxStdDialogButtonSizer();
	okButton = newd wxButton(this, wxID_OK);
	buttonsBoxSizer->AddButton(okButton);
	cancelButton = newd wxButton(this, wxID_CANCEL);
	buttonsBoxSizer->AddButton(cancelButton);
	buttonsBoxSizer->Realize();
	optionsBoxSizer->Add(buttonsBoxSizer, 0, wxALIGN_CENTER | wxALL, 5);

	boxSizer->Add(optionsBoxSizer, 1, wxALL, 5);

	// --------------- Types ---------------

	wxStaticBoxSizer* typeBoxSizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Types"), wxVERTICAL);

	wxString typesChoices[] = { "Depot",
								"Mailbox",
								"Trash Holder",
								"Container",
								"Door",
								"Magic Field",
								"Teleport",
								"Bed",
								"Key" };

	int typesChoicesCount = sizeof(typesChoices) / sizeof(wxString);
	typesRadioBox = newd wxRadioBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, typesChoicesCount, typesChoices, 1, wxRA_SPECIFY_COLS);
	typesRadioBox->SetSelection(0);
	typesRadioBox->Enable(false);
	typeBoxSizer->Add(typesRadioBox, 0, wxALL | wxEXPAND, 5);

	boxSizer->Add(typeBoxSizer, 1, wxALL | wxEXPAND, 5);

	// --------------- Tile Types ---------------

	wxString tileTypesChoices[] = { "PZ",
									"PVP",
									"No PVP",
									"No Logout" };

	int tileTypesChoicesCount = sizeof(tileTypesChoices) / sizeof(wxString);
	tileTypesRadioBox = newd wxRadioBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, tileTypesChoicesCount, tileTypesChoices, 1, wxRA_SPECIFY_COLS);
	tileTypesRadioBox->SetSelection(0);
	tileTypesRadioBox->Enable(false);
	typeBoxSizer->Add(tileTypesRadioBox, 0, wxALL | wxEXPAND, 5);

	// --------------- Properties ---------------

	wxStaticBoxSizer* propertiesBoxSizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Properties"), wxVERTICAL);
	CreatePropertyRadioBoxes(propertiesBoxSizer);
	boxSizer->Add(propertiesBoxSizer, 1, wxALL | wxEXPAND, 5);

	// --------------- Items list ---------------

	wxStaticBoxSizer* resultBoxSizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Result"), wxVERTICAL);
	itemsList = newd FindDialogListBox(resultBoxSizer->GetStaticBox(), wxID_ANY);
	itemsList->SetMinSize(wxSize(230, 512));
	resultBoxSizer->Add(itemsList, 0, wxALL, 5);
	boxSizer->Add(resultBoxSizer, 1, wxALL | wxEXPAND, 5);

	this->SetSizer(boxSizer);
	this->Layout();
	this->Centre(wxBOTH);
	this->EnableProperties(false);
	this->RefreshContentsInternal();

	// Connect Events
	optionsRadioBox->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnOptionChange), nullptr, this);
	itemIdSpin->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(FindItemDialog::OnItemIdChange), nullptr, this);
	itemIdSpin->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(FindItemDialog::OnItemIdChange), nullptr, this);
	nameTextInput->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(FindItemDialog::OnText), nullptr, this);

	typesRadioBox->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnTypeChange), nullptr, this);
	tileTypesRadioBox->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnTypeChange), nullptr, this);

	unpassableRadio->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	movableRadio->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	blockMissilesRadio->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	blockPathfinderRadio->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	readableRadio->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	writeableRadio->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	pickupableRadio->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	stackableRadio->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	rotatableRadio->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	hangableRadio->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	hookEastRadio->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	hookSouthRadio->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	hasElevationRadio->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	ignoreLookRadio->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	floorChangeRadio->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	alwaysOnBottomRadio->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	isGroundRadio->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
}

FindItemDialog::~FindItemDialog() {
	// Disconnect Events
	optionsRadioBox->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnOptionChange), NULL, this);
	itemIdSpin->Disconnect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(FindItemDialog::OnItemIdChange), NULL, this);
	itemIdSpin->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(FindItemDialog::OnItemIdChange), NULL, this);
	nameTextInput->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(FindItemDialog::OnText), NULL, this);

	typesRadioBox->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnTypeChange), nullptr, this);
	tileTypesRadioBox->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnTypeChange), nullptr, this);

	unpassableRadio->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	movableRadio->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	blockMissilesRadio->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	blockPathfinderRadio->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	readableRadio->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	writeableRadio->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	pickupableRadio->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	stackableRadio->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	rotatableRadio->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	hangableRadio->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	hookEastRadio->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	hookSouthRadio->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	hasElevationRadio->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	ignoreLookRadio->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	floorChangeRadio->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	alwaysOnBottomRadio->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
	isGroundRadio->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), nullptr, this);
}

FindItemDialog::SearchMode FindItemDialog::getSearchMode() const {
	return static_cast<SearchMode>(optionsRadioBox->GetSelection());
}

FindItemDialog::SearchTileType FindItemDialog::getSearchTileType() const {
	return static_cast<SearchTileType>(tileTypesRadioBox->GetSelection());
}

void FindItemDialog::setSearchMode(SearchMode mode) {
	if (static_cast<SearchMode>(optionsRadioBox->GetSelection()) != mode) {
		optionsRadioBox->SetSelection(mode);
	}

	itemIdSpin->Enable(mode == SearchMode::ItemIDs);
	nameTextInput->Enable(mode == SearchMode::Names);
	typesRadioBox->Enable(mode == SearchMode::Types);
	tileTypesRadioBox->Enable(mode == SearchMode::TileTypes);
	EnableProperties(mode == SearchMode::Properties);
	RefreshContentsInternal();

	if (mode == SearchMode::ItemIDs) {
		itemIdSpin->SetFocus();
		itemIdSpin->SetSelection(-1, -1);
	} else if (mode == SearchMode::Names) {
		nameTextInput->SetFocus();
	}
}

void FindItemDialog::EnableProperties(bool enable) {
	unpassableRadio->Enable(enable);
	movableRadio->Enable(enable);
	blockMissilesRadio->Enable(enable);
	blockPathfinderRadio->Enable(enable);
	readableRadio->Enable(enable);
	writeableRadio->Enable(enable);
	pickupableRadio->Enable(!onlyPickupables && enable);
	stackableRadio->Enable(enable);
	rotatableRadio->Enable(enable);
	hangableRadio->Enable(enable);
	hookEastRadio->Enable(enable);
	hookSouthRadio->Enable(enable);
	hasElevationRadio->Enable(enable);
	ignoreLookRadio->Enable(enable);
	floorChangeRadio->Enable(enable);
	alwaysOnBottomRadio->Enable(enable);
	isGroundRadio->Enable(enable);
}

void FindItemDialog::RefreshContentsInternal() {
	itemsList->Clear();
	okButton->Enable(false);

	SearchMode selection = (SearchMode)optionsRadioBox->GetSelection();
	bool foundSearchResults = false;

	if (selection == SearchMode::ItemIDs) {
		uint16_t itemID = (uint16_t)itemIdSpin->GetValue();
		for (int id = g_items.getMinID(); id <= g_items.getMaxID(); ++id) {
			const ItemType &item = g_items.getItemType(id);
			if (item.id == 0 || item.id != itemID) {
				continue;
			}

			RAWBrush* rawBrush = item.raw_brush;
			if (!rawBrush) {
				continue;
			}

			if (onlyPickupables && !item.pickupable) {
				continue;
			}

			foundSearchResults = true;
			itemsList->AddBrush(rawBrush);
		}
	} else if (selection == SearchMode::Names) {
		std::string searchString = as_lower_str(nstr(nameTextInput->GetValue()));
		if (searchString.size() >= 2) {
			for (int id = g_items.getMinID(); id <= g_items.getMaxID(); ++id) {
				const ItemType &item = g_items.getItemType(id);
				if (item.id == 0) {
					continue;
				}

				RAWBrush* rawBrush = item.raw_brush;
				if (!rawBrush) {
					continue;
				}

				if (onlyPickupables && !item.pickupable) {
					continue;
				}

				if (as_lower_str(rawBrush->getName()).find(searchString) == std::string::npos) {
					continue;
				}

				foundSearchResults = true;
				itemsList->AddBrush(rawBrush);
			}
		}
	} else if (selection == SearchMode::Types) {
		for (int id = g_items.getMinID(); id <= g_items.getMaxID(); ++id) {
			const ItemType &item = g_items.getItemType(id);
			if (item.id == 0) {
				continue;
			}

			RAWBrush* rawBrush = item.raw_brush;
			if (!rawBrush) {
				continue;
			}

			if (onlyPickupables && !item.pickupable) {
				continue;
			}

			SearchItemType selection = (SearchItemType)typesRadioBox->GetSelection();
			if ((selection == SearchItemType::Depot && !item.isDepot()) || (selection == SearchItemType::Mailbox && !item.isMailbox()) || (selection == SearchItemType::TrashHolder && !item.isTrashHolder()) || (selection == SearchItemType::Container && !item.isContainer()) || (selection == SearchItemType::Door && !item.isDoor()) || (selection == SearchItemType::MagicField && !item.isMagicField()) || (selection == SearchItemType::Teleport && !item.isTeleport()) || (selection == SearchItemType::Bed && !item.isBed()) || (selection == SearchItemType::Key && !item.isKey())) {
				continue;
			}

			foundSearchResults = true;
			itemsList->AddBrush(rawBrush);
		}
	} else if (selection == SearchMode::Properties) {
		ItemFilter filter;
		filter.onlyPickupables = onlyPickupables;

		filter.unpassable = (FilterChoice)unpassableRadio->GetSelection();
		filter.moveable = (FilterChoice)movableRadio->GetSelection();
		filter.blockMissiles = (FilterChoice)blockMissilesRadio->GetSelection();
		filter.blockPathfinder = (FilterChoice)blockPathfinderRadio->GetSelection();
		filter.readable = (FilterChoice)readableRadio->GetSelection();
		filter.writeable = (FilterChoice)writeableRadio->GetSelection();
		filter.pickupable = (FilterChoice)pickupableRadio->GetSelection();
		filter.stackable = (FilterChoice)stackableRadio->GetSelection();
		filter.rotatable = (FilterChoice)rotatableRadio->GetSelection();
		filter.hangable = (FilterChoice)hangableRadio->GetSelection();
		filter.hookEast = (FilterChoice)hookEastRadio->GetSelection();
		filter.hookSouth = (FilterChoice)hookSouthRadio->GetSelection();
		filter.hasElevation = (FilterChoice)hasElevationRadio->GetSelection();
		filter.ignoreLook = (FilterChoice)ignoreLookRadio->GetSelection();
		filter.floorChange = (FilterChoice)floorChangeRadio->GetSelection();
		filter.alwaysOnBottom = (FilterChoice)alwaysOnBottomRadio->GetSelection();
		filter.isGround = (FilterChoice)isGroundRadio->GetSelection();

		if (filter.hasActiveFilters()) {
			auto filteredItems = filter.filterItems();
			for (const ItemType* item : filteredItems) {
				foundSearchResults = true;
				itemsList->AddBrush(item->raw_brush);
			}
		}
	}

	okButton->Enable(foundSearchResults || selection == SearchMode::TileTypes);
	if (foundSearchResults) {
		itemsList->SetSelection(0);
	} else {
		itemsList->SetNoMatches();
	}

	itemsList->Refresh();
}

void FindItemDialog::OnOptionChange(wxCommandEvent &WXUNUSED(event)) {
	setSearchMode(static_cast<SearchMode>(optionsRadioBox->GetSelection()));
}

void FindItemDialog::OnItemIdChange(wxCommandEvent &WXUNUSED(event)) {
	RefreshContentsInternal();
}

void FindItemDialog::OnText(wxCommandEvent &WXUNUSED(event)) {
	inputTimer.Start(800, true);
}

void FindItemDialog::OnTypeChange(wxCommandEvent &WXUNUSED(event)) {
	RefreshContentsInternal();
}

void FindItemDialog::OnPropertyChange(wxCommandEvent &WXUNUSED(event)) {
	RefreshContentsInternal();
}

void FindItemDialog::OnInputTimer(wxTimerEvent &WXUNUSED(event)) {
	RefreshContentsInternal();
}

void FindItemDialog::OnClickOK(wxCommandEvent &WXUNUSED(event)) {
	if (itemsList->GetItemCount() != 0 && !tileTypesRadioBox->IsEnabled()) {
		Brush* brush = itemsList->GetSelectedBrush();
		if (brush) {
			resultBrush = brush;
			resultId = brush->asRaw()->getItemID();
			EndModal(wxID_OK);
			if (!onSelection) {
				g_gui.SelectBrush(brush->asRaw(), TILESET_RAW);
			}
		}
	} else if (tileTypesRadioBox->IsEnabled()) {
		resultBrush = nullptr;
		resultId = 0;
		EndModal(wxID_OK);
	}
}

void FindItemDialog::OnClickCancel(wxCommandEvent &WXUNUSED(event)) {
	EndModal(wxID_CANCEL);
}
