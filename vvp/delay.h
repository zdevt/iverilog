#ifndef __delay_H
#define __delay_H
/*
 * Copyright 2005 Stephen Williams
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifdef HAVE_CVS_IDENT
#ident "$Id: delay.h,v 1.15 2007/03/02 06:13:22 steve Exp $"
#endif

/*
 */

# include  "vvp_net.h"
# include  "schedule.h"

enum delay_edge_t {
      DELAY_EDGE_01 = 0, DELAY_EDGE_10, DELAY_EDGE_0z,
      DELAY_EDGE_z1,     DELAY_EDGE_1z, DELAY_EDGE_z0,
      DELAY_EDGE_0x,     DELAY_EDGE_x1, DELAY_EDGE_1x,
      DELAY_EDGE_x0,     DELAY_EDGE_xz, DELAY_EDGE_zx,
      DELAY_EDGE_COUNT
};

/*
 * Instances of this object are functions that calculate the delay for
 * the transition from the source vvp_bit4_t value to the destination
 * value.
 */
class vvp_delay_t {

    public:
      vvp_delay_t(vvp_time64_t rise, vvp_time64_t fall);
      vvp_delay_t(vvp_time64_t rise, vvp_time64_t fall, vvp_time64_t decay);
      ~vvp_delay_t();

      vvp_time64_t get_delay(vvp_bit4_t from, vvp_bit4_t to);
      vvp_time64_t get_min_delay() const;

      void set_rise(vvp_time64_t val);
      void set_fall(vvp_time64_t val);
      void set_decay(vvp_time64_t val);

    private:
      vvp_time64_t rise_, fall_, decay_;
      vvp_time64_t min_delay_;

      void calculate_min_delay_();
};

/* vvp_fun_delay
 * This is a lighter weight version of vvp_fun_drive, that only
 * carries delays. The output that it propagates is vvp_vector4_t so
 * drive strengths are lost, but then again it doesn't go through the
 * effort of calculating strength values either.
 *
 * The node needs a pointer to the vvp_net_t input so that it knows
 * how to find its output when propaging delayed output.
 *
 * NOTE: This node supports vec4 and real by repeating whatever was
 * input. This is a bit of a hack, as it may be more efficient to
 * create the right type of vvp_fun_delay_real.
 */
class vvp_fun_delay  : public vvp_net_fun_t, private vvp_gen_event_s {

      struct event_ {
	    event_(vvp_time64_t s) : sim_time(s) { }
	    void (vvp_fun_delay::*run_run_ptr)(struct vvp_fun_delay::event_*cur);
	    const vvp_time64_t sim_time;
	    vvp_vector4_t ptr_vec4;
	    vvp_vector8_t ptr_vec8;
	    double ptr_real;
	    struct event_*next;
      };

    public:
      vvp_fun_delay(vvp_net_t*net, vvp_bit4_t init, const vvp_delay_t&d);
      ~vvp_fun_delay();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);
      void recv_vec8(vvp_net_ptr_t port, vvp_vector8_t bit);
      void recv_real(vvp_net_ptr_t port, double bit);
	//void recv_long(vvp_net_ptr_t port, long bit);

    private:
      virtual void run_run();


      void run_run_vec4_(struct vvp_fun_delay::event_*cur);
      void run_run_vec8_(struct vvp_fun_delay::event_*cur);
      void run_run_real_(struct vvp_fun_delay::event_*cur);

    private:
      vvp_net_t*net_;
      vvp_delay_t delay_;

      vvp_vector4_t cur_vec4_;
      vvp_vector8_t cur_vec8_;
      double cur_real_;

      struct event_ *list_;
      void enqueue_(struct event_*cur)
      {
	    if (list_) {
		  cur->next = list_->next;
		  list_->next = cur;
		  list_ = cur;
	    } else {
		  cur->next = cur;
		  list_ = cur;
	    }
      }
      struct event_* dequeue_(void)
      {
	    if (list_ == 0)
		  return 0;
	    struct event_*cur = list_->next;
	    if (list_ == cur)
		  list_ = 0;
	    else
		  list_->next = cur->next;
	    return cur;
      }
      void clean_pulse_events_(vvp_time64_t use_delay);
};

/*
* These objects inplement module delay paths. The fun_modpath functor
* is the output of the modpath, and the vvp_fun_modpath_src is the
* source of the modpath. The modpath source tracks events on the
* inputs to enable delays, and the vvp_fun_modpath, when it's time to
* schedule, looks at the associated modpath_src objects for which
* paths are active.
*/
class vvp_fun_modpath;
class vvp_fun_modpath_src;

class vvp_fun_modpath  : public vvp_net_fun_t, private vvp_gen_event_s {

    public:
      vvp_fun_modpath(vvp_net_t*net);
      ~vvp_fun_modpath();

      void add_modpath_src(vvp_fun_modpath_src*that);

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);

    private:
      virtual void run_run();

    private:
      vvp_net_t*net_;

      vvp_vector4_t cur_vec4_;

      vvp_fun_modpath_src*src_list_;

    private: // not implemented
      vvp_fun_modpath(const vvp_fun_modpath&);
      vvp_fun_modpath& operator= (const vvp_fun_modpath&);
};

class vvp_fun_modpath_src  : public vvp_net_fun_t {

      friend class vvp_fun_modpath;

    public:
      vvp_fun_modpath_src(vvp_time64_t d[12]);
    protected:
      ~vvp_fun_modpath_src();

    public:
      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);

      virtual bool test_vec4(const vvp_vector4_t&bit);

    private:
	// FIXME: Needs to be a 12-value array
      vvp_time64_t delay_[12];
	// Used by vvp_fun_modpath to keep a list of modpath_src objects.
      vvp_fun_modpath_src*next_;

      vvp_time64_t wake_time_;
      bool condition_flag_;

    private:
      vvp_fun_modpath_src(const vvp_fun_modpath_src&);
      vvp_fun_modpath_src& operator = (const vvp_fun_modpath_src&);
};

class vvp_fun_modpath_edge  : public vvp_fun_modpath_src {

    public:
      vvp_fun_modpath_edge(vvp_time64_t del[12], bool pos, bool neg);

      bool test_vec4(const vvp_vector4_t&bit);

    private:
      vvp_bit4_t old_value_;
      bool posedge_;
      bool negedge_;
};

/*
 * $Log: delay.h,v $
 * Revision 1.15  2007/03/02 06:13:22  steve
 *  Add support for edge sensitive spec paths.
 *
 * Revision 1.14  2007/03/01 06:19:39  steve
 *  Add support for conditional specify delay paths.
 *
 * Revision 1.13  2007/01/26 05:15:41  steve
 *  More literal implementation of inertial delay model.
 *
 * Revision 1.12  2006/09/29 03:57:01  steve
 *  Modpath delay chooses correct delay for edge.
 *
 * Revision 1.11  2006/09/23 04:57:20  steve
 *  Basic support for specify timing.
 *
 * Revision 1.10  2006/01/02 05:32:07  steve
 *  Require explicit delay node from source.
 *
 * Revision 1.9  2005/07/06 04:29:25  steve
 *  Implement real valued signals and arith nodes.
 *
 * Revision 1.8  2005/06/22 00:04:49  steve
 *  Reduce vvp_vector4 copies by using const references.
 *
 * Revision 1.7  2005/06/02 16:02:11  steve
 *  Add support for notif0/1 gates.
 *  Make delay nodes support inertial delay.
 *  Add the %force/link instruction.
 *
 * Revision 1.6  2005/05/14 19:43:23  steve
 *  Move functor delays to vvp_delay_fun object.
 *
 * Revision 1.5  2005/04/03 05:45:51  steve
 *  Rework the vvp_delay_t class.
 *
 */
#endif // __delay_H
