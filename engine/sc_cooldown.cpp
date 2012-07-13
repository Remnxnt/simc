// ==========================================================================
// Dedmonwakeen's DPS-DPM Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simulationcraft.hpp"

namespace { // UNNAMED NAMESPACE

struct recharge_event_t : event_t
{
  cooldown_t* cooldown;

  recharge_event_t( player_t* p, cooldown_t* cd, timespan_t delay = timespan_t::zero() ) :
    event_t( p -> sim, p, ( cd -> name_str + "_recharge" ).c_str() ), cooldown( cd )
  {
    sim -> add_event( this, cd -> duration + delay );
  }

  virtual void execute()
  {
    assert( cooldown -> current_charge < cooldown -> charges );
    cooldown -> current_charge++;

    if ( cooldown -> current_charge < cooldown -> charges )
    {
      cooldown -> recharge_event = new ( sim ) recharge_event_t( player, cooldown );
    }
    else
    {
      cooldown -> recharge_event = 0;
    }
  }

};

} // UNNAMED NAMESPACE


void cooldown_t::adjust( timespan_t amount )
{
  ready += amount;

  if ( remains() == timespan_t::zero() )
    reset( true );
}

void cooldown_t::reset( bool early )
{
  ready = ready_init();
  current_charge = charges;
  if ( early )
    reset_react = sim -> current_time + player -> total_reaction_time();
  else
    reset_react = timespan_t::zero();
  event_t::cancel( recharge_event );
}

void cooldown_t::start( timespan_t override, timespan_t delay )
{
  reset_react = timespan_t::zero();
  if ( override >= timespan_t::zero() ) duration = override;
  if ( duration > timespan_t::zero() )
  {
    if ( charges > 1 )
    {
      assert( current_charge > 0 );
      current_charge--;

      if ( current_charge == charges - 1 )
      {
        recharge_event = new ( sim ) recharge_event_t( player, this, delay );
      }
      else if ( current_charge == 0 )
      {
        assert( recharge_event );
        ready = recharge_event -> time + timespan_t::from_millis( 1 );
      }
    }
    else
    {
      ready = sim -> current_time + duration + delay;
    }
  }
}
