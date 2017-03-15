#include "provided.h"
#include "support.h"
#include "MyMap.h"
#include <chrono>
#include <list>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <stack>
using namespace std;

class NavigatorImpl
{
public:
	NavigatorImpl();
	~NavigatorImpl();
	bool loadMapData(string mapFile);
	NavResult navigate(string start, string end, vector<NavSegment>& directions) const;
private:
	
	AttractionMapper am;
	SegmentMapper sm;
	MapLoader ml;
	vector<node> surrond_cord(const node& input) const;
	string angle_calculator(const node& a, const node& b);
	bool reconstruct_path(node& current, MyMap<node, node>& prev,  vector<NavSegment>& direction) const;
	bool geo_included(const node& coord, const std::vector<node>& vec) const;

};

NavigatorImpl::NavigatorImpl()
{
}

NavigatorImpl::~NavigatorImpl()
{
}

bool NavigatorImpl::loadMapData(string mapFile)
{
	cerr << "entering loading " << endl;
	if (ml.load(mapFile))
	{
		cerr << "lm check point 1" << endl;
		am.init(ml);
		sm.init(ml);
		return true;
	}
	else
		return false;
}



vector<node> NavigatorImpl::surrond_cord(const node & input) const
{
	vector<StreetSegment> streets = sm.getSegments(input.node_geo);
	vector<node> cord;
	for (unsigned i = 0; i < streets.size(); i++)
	{

		if (!(input.node_geo == streets[i].segment.end))
		{
			node temp;
			temp.node_geo = streets[i].segment.end;
			temp.street_name = streets[i].streetName;
			cord.push_back(temp);
		}
		if (!(input.node_geo == streets[i].segment.start))
		{
			node temp;
			temp.node_geo = streets[i].segment.start;
			temp.street_name = streets[i].streetName;
			cord.push_back(temp);
		}	
		for (unsigned k = 0; k < streets[i].attractions.size(); k++)
			if (!(input.node_geo == streets[i].attractions[k].geocoordinates))
			{
				node temp;
				temp.node_geo = streets[i].attractions[k].geocoordinates;
				temp.street_name = streets[i].streetName;
				cord.push_back(temp);
			}
	}
	return cord;
}

string NavigatorImpl::angle_calculator(const node & a, const node & b)
{
	GeoSegment temp(a.node_geo, b.node_geo);
	double ang = angleOfLine(temp);
	if (ang >= 0 && ang <= 22.5)
		return "east";
	else if (ang >= 22.5 && ang <= 67.5)
		return "northeast";
	else if (ang >= 67.5 && ang <= 112.5)
		return "north";
	else if (ang >= 112.5 && ang <= 157.5)
		return "northwest";
	else if (ang >= 157.5 && ang <= 202.5)
		return "west";
	else if (ang >= 202.5 && ang <= 247.5)
		return "southwest";
	else if (ang >= 247.5 && ang <= 292.5)
		return "south";
	else if (ang >= 292.5 && ang <= 337.5)
		return "southeast";
	else if (ang >= 337.5 && ang <= 360)
		return "east";

}

bool NavigatorImpl::reconstruct_path(node & current, MyMap<node, node>& prev, vector<NavSegment>& direction) const
{
	stack<node> geo_hld;
	node* previous_cord = &current;

	while (previous_cord != nullptr)
	{
		cerr << previous_cord->node_geo.latitudeText << "," << previous_cord->node_geo.longitudeText << endl;

		geo_hld.push(*previous_cord);
		previous_cord = prev.find(*previous_cord);
	}
	


	return true;
}

bool NavigatorImpl::geo_included(const node & coord, const std::vector<node>& vec) const
{
	
	for (size_t i = 0; i < vec.size(); i++)
	{
		if (vec[i].node_geo == coord.node_geo)
			return true;
	}
	return false;
}


NavResult NavigatorImpl::navigate(string start, string end, vector<NavSegment> &directions) const
{
	////chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

	node start_cord, end_cord;
	if (!am.getGeoCoord(start, start_cord.node_geo))
		return NAV_BAD_SOURCE;
	if (!am.getGeoCoord(end, end_cord.node_geo))
		return NAV_BAD_DESTINATION;

	//should be correct 
	start_cord.street_name = sm.getSegments(start_cord.node_geo).front().streetName;
	end_cord.street_name = sm.getSegments(end_cord.node_geo).front().streetName;


	vector<node> openset;
	MyMap<node, string> closedset;
	//vector<node> closedset;

	openset.push_back(start_cord);

	MyMap<node, node> prevnode;

	MyMap<node, double> gScore;
	gScore.associate(start_cord, 0);
	MyMap<node, double> fScore;
	fScore.associate(start_cord, distanceEarthKM(start_cord.node_geo, end_cord.node_geo));
	//int count = 0;
	while (!openset.empty())
	{
		//cerr << "========================WHILE LOOP=====================" << endl;
		unsigned delete_index = 0;
		node current = openset[0];
		double temp_lowest_f_score = *fScore.find(openset[0]);
		for (int i = 0; i < openset.size();i++)
		{
			//cerr << "========================1st FOR LOOP=====================" << endl;
			double temp_fScore = *fScore.find(openset[i]);
			if (temp_fScore < temp_lowest_f_score)	
				temp_lowest_f_score = temp_fScore;
			if (temp_fScore < temp_lowest_f_score)
				delete_index = i;
		}

		current = openset[delete_index];
		if (current == end_cord)
		{
			if (reconstruct_path(end_cord, prevnode, directions))
				/*chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
				cout << "Time to find route: " << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() << "ms" << endl;*/
				return NAV_SUCCESS;
		}
		openset.erase(openset.begin() + delete_index);
		closedset.associate(current, "abc");

		vector<node> surround = surrond_cord(current);
		
		for (int i = 0; i < surround.size();i++)
		{

			if (closedset.find(surround[i]) != nullptr)

				continue;
			
			double tent_gScore = *gScore.find(current) + distanceEarthKM(current.node_geo, surround[i].node_geo);
			if (!geo_included(surround[i], openset))				
				openset.push_back(surround[i]);
			
			else if (tent_gScore >= *gScore.find(surround[i]))
				continue;

			prevnode.associate(surround[i], current);
			gScore.associate(surround[i], tent_gScore);
			fScore.associate(surround[i], *gScore.find(surround[i]) + distanceEarthKM(surround[i].node_geo, end_cord.node_geo));

		}
		
	}
	
	return NAV_NO_ROUTE;


}

//******************** Navigator functions ************************************

// These functions simply delegate to NavigatorImpl's functions.
// You probably don't want to change any of this code.

Navigator::Navigator()
{
	m_impl = new NavigatorImpl;
}

Navigator::~Navigator()
{
	delete m_impl;
}

bool Navigator::loadMapData(string mapFile)
{
	return m_impl->loadMapData(mapFile);
}

NavResult Navigator::navigate(string start, string end, vector<NavSegment>& directions) const
{
	return m_impl->navigate(start, end, directions);
}
