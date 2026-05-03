#pragma once
#include <vector>
#include <functional>

enum class FilterChoice {
	Any = 0,
	Yes = 1,
	No = 2
};

class ItemFilter {
public:
	FilterChoice unpassable = FilterChoice::Any;
	FilterChoice moveable = FilterChoice::Any;
	FilterChoice blockMissiles = FilterChoice::Any;
	FilterChoice blockPathfinder = FilterChoice::Any;
	FilterChoice readable = FilterChoice::Any;
	FilterChoice writeable = FilterChoice::Any;
	FilterChoice pickupable = FilterChoice::Any;
	FilterChoice stackable = FilterChoice::Any;
	FilterChoice rotatable = FilterChoice::Any;
	FilterChoice hangable = FilterChoice::Any;
	FilterChoice hookEast = FilterChoice::Any;
	FilterChoice hookSouth = FilterChoice::Any;
	FilterChoice hasElevation = FilterChoice::Any;
	FilterChoice ignoreLook = FilterChoice::Any;
	FilterChoice floorChange = FilterChoice::Any;
	FilterChoice alwaysOnBottom = FilterChoice::Any;
	FilterChoice isGround = FilterChoice::Any;

	bool onlyPickupables = false;

	std::vector<const ItemType*> filterItems() const;

	bool hasActiveFilters() const;

private:
	bool passesFilter(const ItemType &item) const;

	bool checkPropertyFilter(FilterChoice filter, bool itemHasProperty) const;
};
