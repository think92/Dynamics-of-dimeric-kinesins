/*****************************************************************************
 *  Purpose:                                                                 *
 *		Dynamics of the kinesin-1 on the microtubule (five dimensions).      *
 *      Limping.                                                             *				
 *****************************************************************************/

#include "nr3.h"                            
#include "ran.h"       
#include "deviates.h"
#include "variable.h"

using namespace std;

int intseed;
int expcount;
const double time_r = 10.0;
const double time_r2 = 50.0;
double Edock_kt;
const double E1_kt = 22.3;
const double E2_kt_before = 32.0;
const double E2_kt_after = 5.0;
const double E3_kt = 35;
const double ita = 2.0E-3;
const double c = 1;
const double rb = 500.0;
double fload;
double fload_y;
double wherelocal;

double xxx(double x);
double abg(double x);
double weak();
double fx_weak();
double fy_weak();
double fz_weak();
double fa_weak();
double fb_weak();
double fg_weak();
double local();
double fx_local();
double fy_local();
double fz_local();
double fa_local();
double fb_local();
double fg_local();
double kk();
double fx_kk();
double fy_kk();
double fz_kk();
double fa_kk();
double fb_kk();
double fg_kk();
double fx_nl();
double fy_nl();
double fz_nl();
double extra();
double fx_extra();
double fy_extra();
double fz_extra();
double fx_dock();
double fx_b();
double fb();
double fy_b();
double fb_y();
void step_local();
void step_weak();

const int sample = 100000;                                     //sample steps:1e5
const double delta_t = 1.0E-5;                    //time step: 0.01 nanosecond
const double d = 8.0; 					          //step length: 8.0 nanometer
const double alpha = 3.0; 					      //weak length: 3.0 nanometer
const double kBT2pNnm = 4.17;                             //kBT->pNnm (T=300K)
const double A = 1.0;                      //interaction distance: 1 nanometer 
const double rh = 2.5;									 //kinesin head radius
const double Pi = 3.1415927;							                  //pi
const double Epsilon = 1.0E-14;
const double theta_hh = 120.0;

int ifdock;
int stepcount;
int stopcount;
int midcount;
int stepnumber;
int stopnumber;
int midnumber;
int step;
double xh0;                                                      //x of head 0 
double yh0;                                               		 //y of head 0
double zh0;														 //z of head 0
double ah0;
double gh0;
double xb;
double yb;
double timer;
double x_temp;
double y_temp;
double z_temp;
double a_temp;
double g_temp;
double xb_temp;
double yb_temp;
double F1_temp;
double F2_temp;
double normal_x;
double normal_y;
double normal_z;
double normal_a;
double normal_g;
double normal_xb;
double normal_yb;
double Ew;
double V0;
double E2_kt;
double t0;
int ifout;
double limping;
double limping_delta;

int main()
{
	string par_temp;
    ifstream parin("../par/in_par.txt");
    parin >> par_temp >> intseed >> par_temp >> expcount >> par_temp >> Edock_kt \
	>> par_temp >> fload >> par_temp >> fload_y >> par_temp >> limping;
    parin.close();
    
    intseed=intseed;Edock_kt=4.5;fload=-4.0;fload_y=3;

    ofstream out_x("../data/x.dat");
    ofstream out_t("../data/t.dat");
	ofstream out_xf("../data/xf.dat");
    ofstream out_tf("../data/tf.dat");    
    ofstream out_x0("../data/x0.dat");
    ofstream out_t0("../data/t0.dat");
    ofstream out_x1("../data/x1.dat");
    ofstream out_t1("../data/t1.dat");
    ofstream out_x0_f("../data/x0f.dat");
    ofstream out_t0_f("../data/t0f.dat");
    ofstream out_x1_f("../data/x1f.dat");
    ofstream out_t1_f("../data/t1f.dat");
		
	Ew = E3_kt*kBT2pNnm;



	Ran myran(intseed);
	long long int myseed;

	stepnumber = 0;
	midnumber = 0;
	stopnumber = 0;
    step = 0;
    double mc_dt = 1E-4;
    double kc = 112;
    double kD = 250.0;
    timer = 0.0;
    double timer2 = 0.0;
    double timeADPT = 0.0;
    double timeADPL = 0.0;
    int state = 1;//ATP----ATP
    while (state != 0){
    	if ((myran.doub() <= kc*mc_dt) || state == 2){
    		state = 0;
    		timer = timer+mc_dt;

			myseed = myran.doub()*1.0E6;Normaldev mynormal_x(0.0,1.0,myseed);
			myseed = myran.doub()*1.0E6;Normaldev mynormal_y(0.0,1.0,myseed);
			myseed = myran.doub()*1.0E6;Normaldev mynormal_z(0.0,1.0,myseed);
			myseed = myran.doub()*1.0E6;Normaldev mynormal_a(0.0,1.0,myseed);
			myseed = myran.doub()*1.0E6;Normaldev mynormal_g(0.0,1.0,myseed);
			myseed = myran.doub()*1.0E6;Normaldev mynormal_xb(0.0,1.0,myseed);
			myseed = myran.doub()*1.0E6;Normaldev mynormal_yb(0.0,1.0,myseed);
		
			timer2 = 0.0;xh0 = -d;yh0 = 0.0;zh0 = 0.0;
			ah0 = 0.0;gh0 = 0.0;xb = -d/2.0+fload/c;yb = fload_y/c/2.0;
			t0 = 0;
	
			ifout = 0;
	
	        wherelocal = 0.0;
	
			ifdock = 0;
			stepcount = 0;
			stopcount = 0;
			midcount = 0;
			
			if (step%2 == 0){
				limping = 0.18;
			}
			else{
				limping = -0.18;
			}

    		while (state == 0){
				for(long long int j=0;;j++)
				{
					
					if(j%100000000 == 0){
						out_x << (xh0/2.0+d/2.0+step*d) << endl;
						out_t << (timer+j*delta_t*1e-6) << endl;
						if(step%2 == 0){
							out_x0 << (xh0/2.0+d/2.0+step*d) << endl;
							out_t0 << (timer+j*delta_t*1e-6) << endl;
						}
						else{
							out_x1 << (xh0/2.0+d/2.0+step*d) << endl;
							out_t1 << (timer+j*delta_t*1e-6) << endl;
						}
					}
					
					if(j%sample == 0){
					if (yh0 <= 1.0&&xh0 <= 0.0)
					{
						stopcount++;
					}
					else if (yh0 <= 1.0&&xh0>0.0)
					{
						stepcount++;
					}
					else
					{
						stepcount = 0; stopcount = 0;
					}
					}

					if(1)
					{
						if(stopcount == 4000)
						{
							timeADPL=timeADPL+1E-6;
							timeADPT=0;
							if(1){
				             	step = step+0;
				             	state = 1;
				             	timer = timer+j*delta_t*1e-6;
				             	timeADPL=0;
								timeADPT=0;
								break;								
							}
						}
						else if(stepcount == 4000)
						{
							timeADPT=timeADPT+1E-6;
							timeADPL=0;
							if(1){
				             	step = step+1;
				             	state = 1;
				             	timer = timer+j*delta_t*1e-6;
				             	if ((myran.doub() <= kc*4e-3)){
				             		state = 2;
								}
								timeADPL=0;
								timeADPT=0;
								break;								
							}
						}
						else{
							timeADPT=0;
							timeADPL=0;
						}
					}
					normal_x=mynormal_x.dev();normal_y=mynormal_y.dev();
					normal_z=mynormal_z.dev();normal_a=mynormal_a.dev();
					normal_g=mynormal_g.dev();normal_xb=mynormal_xb.dev();
					normal_yb=mynormal_yb.dev();
					
					timer2 = j*delta_t;
		
					if(0)
					{
						E2_kt = E2_kt_before;
						V0 = E2_kt*kBT2pNnm;
					}
					else
					{
						E2_kt = E2_kt_after;
						V0 = E2_kt*kBT2pNnm;
						ifdock=1;
					}
					
					if(xh0 < 0)
					{
						limping_delta=limping;
					}
					else
					{
						limping_delta=-limping;
					}
					
					if(timer2 <= time_r)
					{
						step_local();
					}				
					else
					{
						step_weak();
					}
					
				}    			
			}
		}
		else if((myran.doub() <= kc/40.0*mc_dt)){
    		state = 0;
    		timer = timer+mc_dt;

			myseed = myran.doub()*1.0E6;Normaldev mynormal_x(0.0,1.0,myseed);
			myseed = myran.doub()*1.0E6;Normaldev mynormal_y(0.0,1.0,myseed);
			myseed = myran.doub()*1.0E6;Normaldev mynormal_z(0.0,1.0,myseed);
			myseed = myran.doub()*1.0E6;Normaldev mynormal_a(0.0,1.0,myseed);
			myseed = myran.doub()*1.0E6;Normaldev mynormal_g(0.0,1.0,myseed);
			myseed = myran.doub()*1.0E6;Normaldev mynormal_xb(0.0,1.0,myseed);
			myseed = myran.doub()*1.0E6;Normaldev mynormal_yb(0.0,1.0,myseed);
		
			timer2 = 0.0;xh0 = d;yh0 = 0.0;zh0 = 0.0;
			ah0 = 0.0;gh0 = 0.0;xb = d/2.0+fload/c;yb = fload_y/c/2.0;
			t0 = 0;
	
			ifout = 0;
	
	        wherelocal = d*2.0;
	
			ifdock = 0;
			stepcount = 0;
			stopcount = 0;
			midcount = 0;
			
			if (step%2 == 0){
				limping = 0.18;
			}
			else{
				limping = -0.18;
			}

    		while (state == 0){
				for(long long int j=0;;j++)
				{
					
					if(j%100000000 == 0){
						out_x << (xh0/2.0-d/2.0+step*d) << endl;
						out_t << (timer+j*delta_t*1e-6) << endl;
						if(step%2 == 0){
							out_x0 << (xh0/2.0-d/2.0+step*d) << endl;
							out_t0 << (timer+j*delta_t*1e-6) << endl;
						}
						else{
							out_x1 << (xh0/2.0-d/2.0+step*d) << endl;
							out_t1 << (timer+j*delta_t*1e-6) << endl;
						}
					}
					if(j%sample == 0){
					if (yh0 <= 1.0&&xh0 <= 0.0)
					{
						stopcount++;
					}
					else if (yh0 <= 1.0&&xh0>0.0)
					{
						stepcount++;
					}
					else
					{
						stepcount = 0; stopcount = 0;
					}
					}

					if (1)
					{
						if (stopcount == 4000)
						{
							timeADPL = timeADPL + 1E-6;
							timeADPT = 0;
							if (1) {
								step = step + 0;
								state = 1;
								timer = timer + j*delta_t*1e-6;
								timeADPL = 0;
								timeADPT = 0;
								break;
							}
						}
						else if (stepcount == 4000)
						{
							timeADPT = timeADPT + 1E-6;
							timeADPL = 0;
							if (1) {
								step = step + 1;
								state = 1;
								timer = timer + j*delta_t*1e-6;
								if ((myran.doub() <= kc*4e-3)) {
									state = 2;
								}
								timeADPL = 0;
								timeADPT = 0;
								break;
							}
						}
						else {
							timeADPT = 0;
							timeADPL = 0;
						}
					}
					normal_x=mynormal_x.dev();normal_y=mynormal_y.dev();
					normal_z=mynormal_z.dev();normal_a=mynormal_a.dev();
					normal_g=mynormal_g.dev();normal_xb=mynormal_xb.dev();
					normal_yb=mynormal_yb.dev();
					
					timer2 = j*delta_t;
		
					if(0)
					{
						E2_kt = E2_kt_before;
						V0 = E2_kt*kBT2pNnm;
					}
					else
					{
						E2_kt = E2_kt_after;
						V0 = E2_kt*kBT2pNnm;
						ifdock=1;
					}
					
					if(xh0 < 0)
					{
						limping_delta=limping;
					}
					else
					{
						limping_delta=-limping;
					}
					
					if(timer2 <= time_r)
					{
						step_local();
					}				
					else
					{
						step_weak();
					}
					
				}    			
			}			
		}
		else{
			state = 1;
			timer = timer+mc_dt;
		}
		if(1){
			out_x << (step*d) << endl;
			out_t << (timer) << endl;
			out_xf << (step*d) << endl;
			out_tf << (timer) << endl;
			if(step%2 == 0){
				out_x0_f << (step*d) << endl;
				out_t0_f << (timer) << endl;
			}
			else{
				out_x1_f << (step*d) << endl;
				out_t1_f << (timer) << endl;
			}
		}
		if (timer > 10){
			break;
		}
	}
	out_x.close();
	out_t.close();
	out_xf.close();
	out_tf.close();
	out_x0.close();
	out_t0.close();
	out_x1.close();
	out_t1.close();
	out_x0_f.close();
	out_t0_f.close();
	out_x1_f.close();
	out_t1_f.close();
	return 0;
}

double xxx(double x)
{
	if(x > (-(d-alpha))) 
		return  x-((int)((x+d-alpha)/d))*d;
	else 
		return  x+((int)((-x+alpha)/d))*d;
}

double abg(double x)
{
	if(x > (-Pi)) 
		return  x-((int)((x+Pi)/(2.0*Pi)))*(2.0*Pi);
	else 
		return  x+((int)((-x+Pi)/(2.0*Pi)))*(2.0*Pi);
}

double weak()
{
	if(xxx(xh0) > Epsilon) 	
		return  (Ew/(alpha)*xxx(xh0)-Ew) \
				*exp(-fabs(yh0)/A)*exp(-fabs(zh0)/A) \
				*exp(-fabs(abg(ah0))/(A/rh))*exp(-fabs(abg(gh0))/(A/rh));
	else
		return  (-Ew/(d-alpha)*xxx(xh0)-Ew) \
				*exp(-fabs(yh0)/A)*exp(-fabs(zh0)/A) \
				*exp(-fabs(abg(ah0))/(A/rh))*exp(-fabs(abg(gh0))/(A/rh));
}

double fx_weak()
{
	if(xxx(xh0) > Epsilon && xxx(xh0) < alpha)
		return  -Ew/alpha \
				*exp(-fabs(yh0)/A)*exp(-fabs(zh0)/A) \
				*exp(-fabs(abg(ah0))/(A/rh))*exp(-fabs(abg(gh0))/(A/rh));
	else if(xxx(xh0) < -Epsilon && xxx(xh0) > alpha-d)
		return  Ew/(d-alpha) \
				*exp(-fabs(yh0)/A)*exp(-fabs(zh0)/A) \
				*exp(-fabs(abg(ah0))/(A/rh))*exp(-fabs(abg(gh0))/(A/rh));
	else
		return  0.0;
}

double fy_weak()
{
	if(yh0<-Epsilon) 
		return 100.0;
	else if(yh0>Epsilon)
		return  weak()/A;
	else
		return  0.0;
}

double fz_weak()
{
	if(zh0<-Epsilon) 
		return  -weak()/A;
	else if(zh0>Epsilon)
		return  weak()/A;
	else
		return  0.0;
}

double fa_weak()
{
	if(abg(ah0)<-Epsilon && abg(ah0)>-Pi) 
		return  -weak()/(A/rh);
	else if(abg(ah0)>Epsilon && abg(ah0)<Pi)
		return  weak()/(A/rh);
	else
		return  0.0;
}

double fg_weak()
{
	if(abg(gh0)<-Epsilon && abg(gh0)>-Pi) 
		return  -weak()/(A/rh);
	else if(abg(gh0)>Epsilon && abg(gh0)<Pi)
		return  weak()/(A/rh);
	else
		return  0.0;
}

double local()
{
	if(xh0 >= alpha-d-d+wherelocal && xh0 <= alpha-d+wherelocal) 	
		return  weak()/E3_kt*E1_kt;
	else
		return  weak();
}

double fx_local()
{
	if(xh0 >= alpha-d-d+wherelocal && xh0 <= alpha-d+wherelocal) 	
		return  fx_weak()/E3_kt*E1_kt;
	else
		return  fx_weak();
}

double fy_local()
{
	if(yh0<-Epsilon) 
		return 100.0;
	else if(yh0>Epsilon)
		return  local()/A;
	else
		return  0.0;
}

double fz_local()
{
	if(zh0<-Epsilon) 
		return  -local()/A;
	else if(zh0>Epsilon)
		return  local()/A;
	else
		return  0.0;
}

double fa_local()
{
	if(abg(ah0)<-Epsilon && abg(ah0)>-Pi) 
		return  -local()/(A/rh);
	else if(abg(ah0)>Epsilon && abg(ah0)<Pi)
		return  local()/(A/rh);
	else
		return  0.0;
}

double fg_local()
{
	if(abg(gh0)<-Epsilon && abg(gh0)>-Pi) 
		return  -local()/(A/rh);
	else if(abg(gh0)>Epsilon && abg(gh0)<Pi)
		return  local()/(A/rh);
	else
		return  0.0;
}

double kk()
{
	gh0 = gh0 - theta_hh/180.0*Pi;
	double result;
	result = -V0*exp(-pow((xh0-cos((180.0-theta_hh)/180.0*Pi)*(2.0*rh))* \
			 (xh0-cos((180.0-theta_hh)/180.0*Pi)*(2.0*rh))+ \
			 (yh0-sin((180.0-theta_hh)/180.0*Pi)*(2.0*rh))* \
			 (yh0-sin((180.0-theta_hh)/180.0*Pi)*(2.0*rh)) \
			 +zh0*zh0,0.5)/A) \
			 *exp(-fabs(abg(ah0))/(A/rh))*exp(-fabs(abg(gh0))/(A/rh));
	gh0 = gh0 + theta_hh/180.0*Pi;
	return result; 
}

double fx_kk()
{
	if(fabs(xh0-cos((180.0-theta_hh)/180.0*Pi)*(2.0*rh)) < Epsilon) return 0.0;
	else	return kk()*(xh0-cos((180.0-theta_hh)/180.0*Pi)*(2.0*rh))/A/ \
				   (pow((xh0-cos((180.0-theta_hh)/180.0*Pi)*(2.0*rh))* \
			       (xh0-cos((180.0-theta_hh)/180.0*Pi)*(2.0*rh))+ \
		       	   (yh0-sin((180.0-theta_hh)/180.0*Pi)*(2.0*rh))* \
			       (yh0-sin((180.0-theta_hh)/180.0*Pi)*(2.0*rh)) \
			       +zh0*zh0,0.5));
}

double fy_kk()
{
	if(fabs(yh0-sin((180.0-theta_hh)/180.0*Pi)*(2.0*rh)) < Epsilon) return 0.0;
	else	return kk()*(yh0-sin((180.0-theta_hh)/180.0*Pi)*(2.0*rh))/A/ \
				   (pow((xh0-cos((180.0-theta_hh)/180.0*Pi)*(2.0*rh))* \
			       (xh0-cos((180.0-theta_hh)/180.0*Pi)*(2.0*rh))+ \
		       	   (yh0-sin((180.0-theta_hh)/180.0*Pi)*(2.0*rh))* \
			       (yh0-sin((180.0-theta_hh)/180.0*Pi)*(2.0*rh)) \
			       +zh0*zh0,0.5));
}

double fz_kk()
{
	if(fabs(zh0) < Epsilon) return 0.0;
	else	return kk()*(zh0-0.0)/A/ \
				   (pow((xh0-cos((180.0-theta_hh)/180.0*Pi)*(2.0*rh))* \
			       (xh0-cos((180.0-theta_hh)/180.0*Pi)*(2.0*rh))+ \
		       	   (yh0-sin((180.0-theta_hh)/180.0*Pi)*(2.0*rh))* \
			       (yh0-sin((180.0-theta_hh)/180.0*Pi)*(2.0*rh)) \
			       +zh0*zh0,0.5));
}

double fa_kk()
{
	if(abg(ah0)<-Epsilon && abg(ah0)>-Pi) 
		return  -kk()/(A/rh);
	else if(abg(ah0)>Epsilon && abg(ah0)<Pi)
		return  kk()/(A/rh);
	else
		return  0.0;
}

double fg_kk()
{
	double result;
	gh0 = gh0 - theta_hh/180.0*Pi;
	if(abg(gh0)<-Epsilon && abg(gh0)>-Pi) 
	{
		gh0 = gh0 + theta_hh/180.0*Pi;
		result = -kk()/(A/rh);
	}
	else if(abg(gh0)>Epsilon && abg(gh0)<Pi)
	{
		gh0 = gh0 + theta_hh/180.0*Pi;
		result = kk()/(A/rh);
	}
	else
	{
		gh0 = gh0 + theta_hh/180.0*Pi;
		result = 0.0;
	}
	return result;
}

double fx_nl()
{
	if(xh0*xh0+yh0*yh0+zh0*zh0 < Epsilon) return 0.0;
	else
		return 
		-(9.833E-5*exp(3.064*(pow(xh0*xh0+yh0*yh0+zh0*zh0,0.5)+limping_delta)/2.0)) \
		*xh0/(pow(xh0*xh0+yh0*yh0+zh0*zh0,0.5));
}

double fy_nl()
{
	if(xh0*xh0+yh0*yh0+zh0*zh0 < Epsilon) return 0.0;
	else
		return 
		-(9.833E-5*exp(3.064*(pow(xh0*xh0+yh0*yh0+zh0*zh0,0.5)+limping_delta)/2.0)) \
		*yh0/(pow(xh0*xh0+yh0*yh0+zh0*zh0,0.5));
}

double fz_nl()
{
	if(xh0*xh0+yh0*yh0+zh0*zh0 < Epsilon) return 0.0;
	else
		return 
		-(9.833E-5*exp(3.064*(pow(xh0*xh0+yh0*yh0+zh0*zh0,0.5)+limping_delta)/2.0)) \
		*zh0/(pow(xh0*xh0+yh0*yh0+zh0*zh0,0.5));
}

double extra()
{
	if(xh0*xh0+yh0*yh0+zh0*zh0<4*rh*rh) return 100.0;
	else return 0.0;
}

double fx_extra()
{
	return extra()*xh0/(pow(xh0*xh0+yh0*yh0+zh0*zh0,0.5));
}

double fy_extra()
{
	return extra()*yh0/(pow(xh0*xh0+yh0*yh0+zh0*zh0,0.5));
}

double fz_extra()
{
	return extra()*zh0/(pow(xh0*xh0+yh0*yh0+zh0*zh0,0.5));
}

double fx_dock()
{
	if(ifdock == 0) return 0;
	if(ifdock == 1) 
	{
		if(xh0>=0.0&&xh0<=1.0) return Edock_kt*kBT2pNnm/1.0;
		else return 0;
	}
}

double fx_b() //bead to head
{
	if(xh0<=0.0)
	{
		if(xb>xh0/2.0)
		{
			if(fabs(xh0-xb)<=d/2.0)
			{
				return 0.0;
			}
			else if(xh0>xb)
			{
				return -c*(xh0-xb-d/2.0);
			}
			else
			{
				return c*(xb-xh0-d/2.0);
			}
		}
		else
		{
			return 0.0;
		}
	}
	else
	{
		if(xb>xh0/2.0)
		{
			return 0.0;
		}
		else
		{
			if(fabs(xh0-xb)<=d/2.0)
			{
				return 0.0;
			}
			else if(xh0>xb)
			{
				return -c*(xh0-xb-d/2.0);
			}
			else
			{
				return c*(xb-xh0-d/2.0);
			}
		}
	}
}

double fb() //head to bead
{
	if(xh0<=0.0)
	{
		if(xb>xh0/2.0)
		{
			if(fabs(xh0-xb)<=d/2.0)
			{
				return 0.0;
			}
			else if(xh0>xb)
			{
				return c*(xh0-xb-d/2.0);
			}
			else
			{
				return -c*(xb-xh0-d/2.0);
			}
		}
		else
		{
			if(fabs(0.0-xb)<=d/2.0)
			{
				return 0.0;
			}
			else if(0.0>xb)
			{
				return c*(0.0-xb-d/2.0);
			}
			else
			{
				return -c*(xb-0.0-d/2.0);
			}
		}
	}
	else
	{
		if(xb>xh0/2.0)
		{
			if(fabs(0.0-xb)<=d/2.0)
			{
				return 0.0;
			}
			else if(0.0>xb)
			{
				return c*(0.0-xb-d/2.0);
			}
			else
			{
				return -c*(xb-0.0-d/2.0);
			}

		}
		else
		{
			if(fabs(xh0-xb)<=d/2.0)
			{
				return 0.0;
			}
			else if(xh0>xb)
			{
				return c*(xh0-xb-d/2.0);
			}
			else
			{
				return -c*(xb-xh0-d/2.0);
			}
		}
	}	
}

double fy_b() //bead to head
{
	if(yh0<=1.0)
	{
		return c*(yb-yh0);
	}
	else
	{
		return 0;
	}
}

double fb_y()
{
	if(yh0<=1.0)
	{
		return -c*(yb-yh0)-c*(yb-0);
	}
	else
	{
		return -c*(yb-0);
	}
}

void step_local()
{
//	normal_x = mynormal_x.dev();
	F1_temp = (fx_local()+fx_kk()+fx_nl()+fx_extra()+fx_dock()+fx_b())/(6.0*Pi*ita*rh);
	x_temp = xh0;
	xh0 = xh0 + delta_t*F1_temp + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rh)*delta_t,0.5) * normal_x;
	F2_temp = (fx_local()+fx_kk()+fx_nl()+fx_extra()+fx_dock()+fx_b())/(6.0*Pi*ita*rh);
	xh0 = x_temp;
	xh0 = xh0 + 0.5 * delta_t * (F1_temp + F2_temp) + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rh)*delta_t,0.5) * normal_x;
	
//	normal_y = mynormal_y.dev();
	F1_temp = (fy_local()+fy_kk()+fy_nl()+fy_extra()+fy_b())/(6.0*Pi*ita*rh);
	y_temp = yh0;
	yh0 = yh0 + delta_t*F1_temp + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rh)*delta_t,0.5) * normal_y;
	F2_temp = (fy_local()+fy_kk()+fy_nl()+fy_extra()+fy_b())/(6.0*Pi*ita*rh);
	yh0 = y_temp;
	yh0 = yh0 + 0.5 * delta_t * (F1_temp + F2_temp) + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rh)*delta_t,0.5) * normal_y;

//	normal_z = mynormal_z.dev();
	F1_temp = (fz_local()+fz_kk()+fz_nl()+fz_extra())/(6.0*Pi*ita*rh);
	z_temp = zh0;
	zh0 = zh0 + delta_t*F1_temp + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rh)*delta_t,0.5) * normal_z;
	F2_temp = (fz_local()+fz_kk()+fz_nl()+fz_extra())/(6.0*Pi*ita*rh);
	zh0 = z_temp;
	zh0 = zh0 + 0.5 * delta_t * (F1_temp + F2_temp) + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rh)*delta_t,0.5) * normal_z;
	
//	normal_a = mynormal_a.dev();
	F1_temp = (fa_local()+fa_kk())/(8.0*Pi*ita*rh*rh*rh);
	a_temp = ah0;
	ah0 = ah0 + delta_t*F1_temp + \
	pow(2.0*kBT2pNnm/(8.0*Pi*ita*rh*rh*rh)*delta_t,0.5) * normal_a;
	F2_temp = (fa_local()+fa_kk())/(8.0*Pi*ita*rh*rh*rh);
	ah0 = a_temp;
	ah0 = ah0 + 0.5 * delta_t * (F1_temp + F2_temp) + \
	pow(2.0*kBT2pNnm/(8.0*Pi*ita*rh*rh*rh)*delta_t,0.5) * normal_a;

//	normal_g = mynormal_g.dev();
	F1_temp = (fg_local()+fg_kk())/(8.0*Pi*ita*rh*rh*rh);
	g_temp = gh0;
	gh0 = gh0 + delta_t*F1_temp + \
	pow(2.0*kBT2pNnm/(8.0*Pi*ita*rh*rh*rh)*delta_t,0.5) * normal_g;
	F2_temp = (fg_local()+fg_kk())/(8.0*Pi*ita*rh*rh*rh);
	gh0 = g_temp;
	gh0 = gh0 + 0.5 * delta_t * (F1_temp + F2_temp) + \
	pow(2.0*kBT2pNnm/(8.0*Pi*ita*rh*rh*rh)*delta_t,0.5) * normal_g;

//	bead
	F1_temp = (fb()+fload)/(6.0*Pi*ita*rb);
	xb_temp = xb;
	xb = xb + delta_t*F1_temp + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rb)*delta_t,0.5) * normal_xb;
	F2_temp = (fb()+fload)/(6.0*Pi*ita*rb);
	xb = xb_temp;
	xb = xb + 0.5 * delta_t * (F1_temp + F2_temp) + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rb)*delta_t,0.5) * normal_xb;
	
	F1_temp = (fb_y()+fload_y)/(6.0*Pi*ita*rb);
	yb_temp = yb;
	yb = yb + delta_t*F1_temp + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rb)*delta_t,0.5) * normal_yb;
	F2_temp = (fb_y()+fload_y)/(6.0*Pi*ita*rb);
	yb = yb_temp;
	yb = yb + 0.5 * delta_t * (F1_temp + F2_temp) + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rb)*delta_t,0.5) * normal_yb;
}

void step_weak()
{
//	normal_x = mynormal_x.dev();
	F1_temp = (fx_weak()+fx_kk()+fx_nl()+fx_extra()+fx_dock()+fx_b())/(6.0*Pi*ita*rh);
	x_temp = xh0;
	xh0 = xh0 + delta_t*F1_temp + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rh)*delta_t,0.5) * normal_x;
	F2_temp = (fx_weak()+fx_kk()+fx_nl()+fx_extra()+fx_dock()+fx_b())/(6.0*Pi*ita*rh);
	xh0 = x_temp;
	xh0 = xh0 + 0.5 * delta_t * (F1_temp + F2_temp) + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rh)*delta_t,0.5) * normal_x;
	
//	normal_y = mynormal_y.dev();
	F1_temp = (fy_weak()+fy_kk()+fy_nl()+fy_extra()+fy_b())/(6.0*Pi*ita*rh);
	y_temp = yh0;
	yh0 = yh0 + delta_t*F1_temp + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rh)*delta_t,0.5) * normal_y;
	F2_temp = (fy_weak()+fy_kk()+fy_nl()+fy_extra()+fy_b())/(6.0*Pi*ita*rh);
	yh0 = y_temp;
	yh0 = yh0 + 0.5 * delta_t * (F1_temp + F2_temp) + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rh)*delta_t,0.5) * normal_y;

//	normal_z = mynormal_z.dev();
	F1_temp = (fz_weak()+fz_kk()+fz_nl()+fz_extra())/(6.0*Pi*ita*rh);
	z_temp = zh0;
	zh0 = zh0 + delta_t*F1_temp + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rh)*delta_t,0.5) * normal_z;
	F2_temp = (fz_weak()+fz_kk()+fz_nl()+fz_extra())/(6.0*Pi*ita*rh);
	zh0 = z_temp;
	zh0 = zh0 + 0.5 * delta_t * (F1_temp + F2_temp) + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rh)*delta_t,0.5) * normal_z;
	
//	normal_a = mynormal_a.dev();
	F1_temp = (fa_weak()+fa_kk())/(8.0*Pi*ita*rh*rh*rh);
	a_temp = ah0;
	ah0 = ah0 + delta_t*F1_temp + \
	pow(2.0*kBT2pNnm/(8.0*Pi*ita*rh*rh*rh)*delta_t,0.5) * normal_a;
	F2_temp = (fa_weak()+fa_kk())/(8.0*Pi*ita*rh*rh*rh);
	ah0 = a_temp;
	ah0 = ah0 + 0.5 * delta_t * (F1_temp + F2_temp) + \
	pow(2.0*kBT2pNnm/(8.0*Pi*ita*rh*rh*rh)*delta_t,0.5) * normal_a;

//	normal_g = mynormal_g.dev();
	F1_temp = (fg_weak()+fg_kk())/(8.0*Pi*ita*rh*rh*rh);
	g_temp = gh0;
	gh0 = gh0 + delta_t*F1_temp + \
	pow(2.0*kBT2pNnm/(8.0*Pi*ita*rh*rh*rh)*delta_t,0.5) * normal_g;
	F2_temp = (fg_weak()+fg_kk())/(8.0*Pi*ita*rh*rh*rh);
	gh0 = g_temp;
	gh0 = gh0 + 0.5 * delta_t * (F1_temp + F2_temp) + \
	pow(2.0*kBT2pNnm/(8.0*Pi*ita*rh*rh*rh)*delta_t,0.5) * normal_g;

//	bead
	F1_temp = (fb()+fload)/(6.0*Pi*ita*rb);
	xb_temp = xb;
	xb = xb + delta_t*F1_temp + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rb)*delta_t,0.5) * normal_xb;
	F2_temp = (fb()+fload)/(6.0*Pi*ita*rb);
	xb = xb_temp;
	xb = xb + 0.5 * delta_t * (F1_temp + F2_temp) + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rb)*delta_t,0.5) * normal_xb;
	
	F1_temp = (fb_y()+fload_y)/(6.0*Pi*ita*rb);
	yb_temp = yb;
	yb = yb + delta_t*F1_temp + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rb)*delta_t,0.5) * normal_yb;
	F2_temp = (fb_y()+fload_y)/(6.0*Pi*ita*rb);
	yb = yb_temp;
	yb = yb + 0.5 * delta_t * (F1_temp + F2_temp) + \
	pow(2.0*kBT2pNnm/(6.0*Pi*ita*rb)*delta_t,0.5) * normal_yb;
}
