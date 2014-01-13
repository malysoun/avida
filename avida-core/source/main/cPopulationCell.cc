/*
 *  cPopulationCell.cc
 *  Avida
 *
 *  Called "pop_cell.cc" prior to 12/5/05.
 *  Copyright 1999-2011 Michigan State University. All rights reserved.
 *  Copyright 1993-2003 California Institute of Technology.
 *
 *
 *  This file is part of Avida.
 *
 *  Avida is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  Avida is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License along with Avida.
 *  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "cPopulationCell.h"

#include "avida/core/Feedback.h"
#include "cOrganism.h"
#include "cWorld.h"
#include "cEnvironment.h"
#include "cPopulation.h"

#include <cmath>
#include <iterator>

using namespace std;


cPopulationCell::cPopulationCell(const cPopulationCell& in_cell)
: m_world(in_cell.m_world)
, m_organism(in_cell.m_organism)
, m_hardware(in_cell.m_hardware)
, m_inputs(in_cell.m_inputs)
, m_cell_id(in_cell.m_cell_id)
, m_cell_data(in_cell.m_cell_data)
{
  // Copy the mutation rates into a new structure
  m_mut_rates = new cMutationRates(*in_cell.m_mut_rates);
	
  // Copy the connection list
  tConstListIterator<cPopulationCell> conn_it(in_cell.m_connections);
  cPopulationCell* test_cell;
  while ((test_cell = const_cast<cPopulationCell*>(conn_it.Next()))) m_connections.PushRear(test_cell);
	
}

void cPopulationCell::operator=(const cPopulationCell& in_cell)
{
	if (this != &in_cell) {
		m_world = in_cell.m_world;
		m_organism = in_cell.m_organism;
		m_hardware = in_cell.m_hardware;
		m_inputs = in_cell.m_inputs;
		m_cell_id = in_cell.m_cell_id;
		m_cell_data = in_cell.m_cell_data;
		
		// Copy the mutation rates, constructing the structure as necessary
		if (m_mut_rates == NULL)
			m_mut_rates = new cMutationRates(*in_cell.m_mut_rates);
		else
			m_mut_rates->Copy(*in_cell.m_mut_rates);
		
		// Copy the connection list
		tConstListIterator<cPopulationCell> conn_it(in_cell.m_connections);
		cPopulationCell* test_cell;
		while ((test_cell = const_cast<cPopulationCell*>(conn_it.Next()))) m_connections.PushRear(test_cell);
}

void cPopulationCell::Setup(cWorld* world, int in_id, const cMutationRates& in_rates, int x, int y)
{
  m_world = world;
  m_cell_id = in_id;
  m_x = x;
  m_y = y;
  m_cell_data.contents = 0;
  m_cell_data.org_id = -1;
  m_cell_data.update = -1;
  m_cell_data.territory = -1;
  m_spec_state = 0;
  
  if (m_mut_rates == NULL)
    m_mut_rates = new cMutationRates(in_rates);
  else
    m_mut_rates->Copy(in_rates);
}

void cPopulationCell::Rotate(cPopulationCell& new_facing)
{
  // @CAO Note, this breaks avida if new_facing is not in connection_list
	
#ifdef DEBUG
  int scan_count = 0;
#endif
  while (m_connections.GetFirst() != &new_facing) {
    m_connections.CircNext();
#ifdef DEBUG
    assert(++scan_count < m_connections.GetSize());
#endif
  }
}

/*! This method recursively builds a set of cells that neighbor this cell, out to 
 the given depth.  The set must be passed in by-reference, as calls to this method 
 must share a common set of already-visited cells.
 */
void cPopulationCell::GetNeighboringCells(std::set<cPopulationCell*>& cell_set, int depth) const {
	typedef std::set<cPopulationCell*> cell_set_t;
  
  // For each cell in our connection list...
  tConstListIterator<cPopulationCell> i(m_connections);
  while(!i.AtEnd()) {
		// store the cell pointer, and check to see if we've already visited that cell...
    cPopulationCell* cell = i.Next();
		assert(cell != 0); // cells should never be null.
		std::pair<cell_set_t::iterator, bool> ins = cell_set.insert(cell);
		// and if so, recurse to it...
		if(ins.second && (depth > 1)) {
			cell->GetNeighboringCells(cell_set, depth-1);
		}
	}
}

/*! Recursively build a set of occupied cells that neighbor this one, out to the given depth.
*/
void cPopulationCell::GetOccupiedNeighboringCells(std::set<cPopulationCell*>& occupied_cell_set, int depth) const {
	// we'll do this the easy way, and just filter the neighbor set.
	std::set<cPopulationCell*> cell_set;
	GetNeighboringCells(cell_set, depth);
	for(std::set<cPopulationCell*>::iterator i=cell_set.begin(); i!=cell_set.end(); ++i) {
		if((*i)->IsOccupied()) {
			occupied_cell_set.insert(*i);
		}
	}
}

void cPopulationCell::GetOccupiedNeighboringCells(Apto::Array<cPopulationCell*>& occupied_cells) const
{
  occupied_cells.Resize(m_connections.GetSize());
  int occupied_count = 0;

  tLWConstListIterator<cPopulationCell> i(m_connections);
  while(!i.AtEnd()) {
    cPopulationCell* cell = i.Next();
		assert(cell); // cells should never be null.
    if (cell->IsOccupied()) occupied_cells[occupied_count++] = cell;
  }
  
  occupied_cells.Resize(occupied_count);
}


/*! These values are chosen so as to make loops on the facing 'easy'.
 111 = NE
 101 = E
 100 = SE
 000 = S
 001 = SW
 011 = W
 010 = NW
 110 = N
 
 Facing is determined by the relative positions of this cell and the cell that 
 is currently faced. Note that we cannot differentiate between directions on a 2x2 
 torus.
 */
int cPopulationCell::GetFacing()
{
  // This whole function is a hack.
	cPopulationCell* faced = ConnectionList().GetFirst();
	
	int x=0,y=0,lr=0,du=0;
	faced->GetPosition(x,y);
  
	if((x==m_x-1) || (x>m_x+1))
		lr = -1; //left
	else if((x==m_x+1) || (x<m_x-1))
		lr = 1; //right
	
	if((y==m_y-1) || (y>m_y+1))
		du = -1; //up
	else if((y==m_y+1) || (y<m_y-1))
		du = 1; //down
  
	// This is hackish.
	// If you change these return values then the directional send tasks, like sent-north, need to be updated.
	if(lr==0 && du==-1) return 0; //N
	else if(lr==-1 && du==-1) return 1; //NW
	else if(lr==-1 && du==0) return 3; //W
	else if(lr==-1 && du==1) return 2; //SW
	else if(lr==0 && du==1) return 6; //S
	else if(lr==1 && du==1) return 7; //SE
	else if(lr==1 && du==0) return 5; //E
	else if(lr==1 && du==-1) return 4; //NE
  
	assert(false);
  
  return 0;
}

int cPopulationCell::GetFacedDir()
{
  const int facing = GetFacing();
  int faced_dir = 0;    
  if (facing == 0) faced_dir = 0;          //N 
  else if (facing == 1) faced_dir = 7;    //NW
  else if (facing == 3) faced_dir = 6;    //W
  else if (facing == 2) faced_dir = 5;    //SW
  else if (facing == 6) faced_dir = 4;     //S
  else if (facing == 7) faced_dir = 3;     //SE
  else if (facing == 5) faced_dir = 2;     //E
  else if (facing == 4) faced_dir = 1;     //NE
  return faced_dir;
  
}
void cPopulationCell::ResetInputs(cAvidaContext& ctx) 
{ 
  m_world->GetEnvironment().SetupInputs(ctx, m_inputs); 
}


void cPopulationCell::InsertOrganism(cOrganism* new_org, cAvidaContext& ctx) 
{
  assert(new_org != NULL);
  assert(m_organism == NULL);
	
  // Adjust this cell's attributes to account for the new organism.
  m_organism = new_org;
  m_hardware = &new_org->GetHardware();
  m_world->GetStats().AddSpeculativeWaste(m_spec_state);
  m_spec_state = 0;
	
  // Adjust the organism's attributes to match this cell.
  m_organism->GetOrgInterface().SetCellID(m_cell_id);	
}

cOrganism * cPopulationCell::RemoveOrganism(cAvidaContext& ctx) 
{
  if (m_organism == NULL) return NULL;   // Nothing to do!
	
  // For the moment, the cell doesn't keep track of much...
  cOrganism * out_organism = m_organism;
  m_organism = NULL;
  m_hardware = NULL;
  return out_organism;
}