#include "main.h"
#include "item_filter.h"
#include "items.h"

std::vector<const ItemType*> ItemFilter::filterItems() const {
	std::vector<const ItemType*> results;

	for (int id = g_items.getMinID(); id <= g_items.getMaxID(); ++id) {
		const ItemType &item = g_items.getItemType(id);

		if (item.id == 0) {
			continue;
		}

		if (!item.raw_brush) {
			continue;
		}

		if (onlyPickupables && !item.pickupable) {
			continue;
		}

		if (passesFilter(item)) {
			results.push_back(&item);
		}
	}

	return results;
}

bool ItemFilter::hasActiveFilters() const {
	// TODO: use reflection
	const FilterChoice fields[] = {
		unpassable, moveable, blockMissiles, blockPathfinder,
		readable, writeable, pickupable, stackable,
		rotatable, hangable, hookEast, hookSouth,
		hasElevation, ignoreLook, floorChange,
		alwaysOnBottom, isGround
	};
	for (auto f : fields) {
		if (f != FilterChoice::Any) {
			return true;
		}
	}
	return false;
}

bool ItemFilter::passesFilter(const ItemType &item) const {
	if (!checkPropertyFilter(unpassable, item.unpassable)) {
		return false;
	}
	if (!checkPropertyFilter(moveable, item.moveable)) {
		return false;
	}
	if (!checkPropertyFilter(blockMissiles, item.blockMissiles)) {
		return false;
	}
	if (!checkPropertyFilter(blockPathfinder, item.blockPathfinder)) {
		return false;
	}
	if (!checkPropertyFilter(readable, item.canReadText)) {
		return false;
	}
	if (!checkPropertyFilter(writeable, item.canWriteText)) {
		return false;
	}
	if (!checkPropertyFilter(pickupable, item.pickupable)) {
		return false;
	}
	if (!checkPropertyFilter(stackable, item.stackable)) {
		return false;
	}
	if (!checkPropertyFilter(rotatable, item.rotable)) {
		return false;
	}
	if (!checkPropertyFilter(hangable, item.isHangable)) {
		return false;
	}
	if (!checkPropertyFilter(hookEast, item.hookEast)) {
		return false;
	}
	if (!checkPropertyFilter(hookSouth, item.hookSouth)) {
		return false;
	}
	if (!checkPropertyFilter(hasElevation, item.hasElevation)) {
		return false;
	}
	if (!checkPropertyFilter(ignoreLook, item.ignoreLook)) {
		return false;
	}
	if (!checkPropertyFilter(floorChange, item.isFloorChange())) {
		return false;
	}
	if (!checkPropertyFilter(alwaysOnBottom, item.alwaysOnBottom)) {
		return false;
	}
	if (!checkPropertyFilter(isGround, item.group == ITEM_GROUP_GROUND)) {
		return false;
	}

	return true;
}

bool ItemFilter::checkPropertyFilter(FilterChoice filter, bool itemHasProperty) const {
	switch (filter) {
		case FilterChoice::Any:
			return true;
		case FilterChoice::Yes:
			return itemHasProperty;
		case FilterChoice::No:
			return !itemHasProperty;
		default:
			return true;
	}
}
