#pragma once
#include "main.h"
#include "position.h"
#include <map>
#include <set>
class ZoneOccupiedPositions {
public:
	static void addPosition(unsigned int zone, const Position &pos) {
		auto &positions = zonePositions[zone];
		positions.insert(pos);
		calculateCenter(zone);
	}

	static void removePosition(unsigned int zone, const Position &pos) {
		auto it = zonePositions.find(zone);
		if (it != zonePositions.end()) {
			it->second.erase(pos);
			if (it->second.empty()) {
				zonePositions.erase(it);
			} else {
				calculateCenter(zone);
			}
		}
	}

	static bool hasCenter(unsigned int zone) {
		return centerPositions.find(zone) != centerPositions.end();
	}

	static Position getCenter(unsigned int zone) {
		auto it = centerPositions.find(zone);
		if (it != centerPositions.end()) {
			return it->second;
		}
		throw std::runtime_error("Center position not available");
	}

private:
	static inline std::map<unsigned int, std::set<Position>> zonePositions;
	static inline std::map<unsigned int, Position> centerPositions;
	// std::map<unsigned int, Position> ZoneOccupiedPositions::centerPositions;
	// std::map<unsigned int, std::set<Position>> ZoneOccupiedPositions::zonePositions;

	static void calculateCenter(unsigned int zone) {
		auto it = zonePositions.find(zone);
		if (it == zonePositions.end() || it->second.empty()) {
			centerPositions.erase(zone);
			return;
		}

		float sumX = 0, sumY = 0;
		for (const auto &pos : it->second) {
			sumX += pos.x;
			sumY += pos.y;
		}
		float centerX = sumX / it->second.size();
		float centerY = sumY / it->second.size();

		const Position* bestPos = nullptr;
		float bestDistance = std::numeric_limits<float>::max();
		for (const auto &pos : it->second) {
			float distance = std::pow(pos.x - centerX, 2) + std::pow(pos.y - centerY, 2);
			if (distance < bestDistance) {
				bestDistance = distance;
				bestPos = &pos;
			}
		}
		if (bestPos) {
			centerPositions[zone] = *bestPos;
		}
	}
};
