//Implementation file for Shop Class

//-----------------------------Shop.cpp----------------------------------------------------------------------

#include "Shop.h"

//-------------------------------init------------------------------------------------------------------
//Initializer
//Prepares shop before opening to customers
void Shop::init()
{
   pthread_mutex_init(&mutex_, NULL);
   cond_customers_waiting_ = new pthread_cond_t[maxProgrammedNumCust];
   cond_customer_served_ = new pthread_cond_t[maxProgrammedNumCust];
   for(int i = 0; i < maxProgrammedNumCust; i++) {
      pthread_cond_init(&(cond_customer_served_[i]), NULL);
      pthread_cond_init(&(cond_customers_waiting_[i]), NULL);
   }
   cond_barber_paid_ = new pthread_cond_t[max_barber_number];
   cond_barber_sleeping_ = new pthread_cond_t[max_barber_number];
   customer_in_barb_chair_ = new int[max_barber_number];
   in_service_ = new bool[max_barber_number];
   money_paid_  = new bool[max_barber_number];
   for (int i = 0; i < max_barber_number; i++)
   {
      customer_in_barb_chair_[i] = 0; //start shop with all barbers' chairs empty
      ready_barbers.push(i); //place all barbers into queue, smallest to largest
      in_service_[i] = false;
      money_paid_[i] = false;
      pthread_cond_init(&(cond_barber_paid_[i]), NULL);
      pthread_cond_init(&(cond_barber_sleeping_[i]), NULL);
   }
}

//-----------------------------------Destuructor-------------------------------------------------
Shop::~Shop() {
   delete[] cond_barber_paid_;
   delete[] cond_barber_sleeping_;
   delete[] cond_customer_served_;
   delete[] cond_customers_waiting_;
   delete[] customer_in_barb_chair_;
   delete[] in_service_;
   delete[] money_paid_;
}

//--------------------------------int2string---------------------------------------------------------------------
//Helper method of changing int to string, returns string
string Shop::int2string(int i)
{
   stringstream out;
   out << i;
   return out.str();
}

//------------------------------------print--------------------------------------------------------------------------------
//Helper for printing updates
void Shop::print(bool isBarber, int id, string message)
{
   cout << ((isBarber != true) ? "customer[" : "barber  [") << id << "]: " << message << endl;
}

//---------------------------------get_cust_drops--------------------------------------------------------------------------
//Returns number of customer drops because no more waiting room seats
int Shop::get_cust_drops() const
{
   return cust_drops_;
}

//--------------------------------barbersAllBusy------------------------------------------------------------------------------------
//Helper method that returns as a bool whether all barbers are busy
bool Shop::barbersAllBusy()
{
   for (int i = 0; i < max_barber_number; i++)
   {
      if (customer_in_barb_chair_[i] == 0)
      {
         return false;
      }
   }
   return true;
}

//-----------------------------------visitShop-------------------------------------------------------------------------------------------------
//Customer visits shop, requires int custId for parameter
//Returns barbId as an int
int Shop::visitShop(int custId)
{
   int this_cust_barberId = -1;
   pthread_mutex_lock(&mutex_);

   // If all chairs are full then leave shop
   if (waiting_chairs_.size() == max_waiting_cust_)
   {
      print(false, custId, "leaves the shop because of no available waiting chairs.");
      ++cust_drops_;
      pthread_mutex_unlock(&mutex_);
      return -1;
   }

   // All barbers chairs are full or the waiting room is not empty
   if (barbersAllBusy() || waiting_chairs_.size() > 0)
   {
      waiting_chairs_.push(custId); // push customer onto the queue
      print(false, custId, "takes a waiting chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
      barbers_next_cust.push(custId); // add to barbers list queue
      count++; //taking count of if barber needs to check waiting list first
      pthread_cond_wait(&(cond_customers_waiting_[custId]), &mutex_); // waiting until cond customers waiting is signaled
      waiting_chairs_.pop(); // out of waiting room
   }

   this_cust_barberId = ready_barbers.front(); //customer's barber is the first from ready_barbers queue
   customer_in_barb_chair_[this_cust_barberId] = custId; //customer id is put into barber's chair
   ready_barbers.pop();
   print(false, custId, "moves to the service chair [" + int2string(this_cust_barberId) + "], # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
   in_service_[this_cust_barberId] = true;

   // wake up the barber just in case if he is sleeping
   pthread_cond_signal(&(cond_barber_sleeping_[this_cust_barberId]));

   pthread_mutex_unlock(&mutex_);
   return this_cust_barberId;
}

//-------------------------------------------leaveShop----------------------------------------------------------------------------------------------------------------------------------------
//Customer leaves shop, requires custId and barbId for parameters
void Shop::leaveShop(int custId, int barbId)
{
   pthread_mutex_lock(&mutex_);
   print(false, custId, "wait for barber [" + int2string(barbId) + "] to be done with hair-cut");
   // Customer waits for service to be completed
   while (in_service_[barbId] == true)
   {
      pthread_cond_wait(&(cond_customer_served_[custId]), &mutex_); 
   }

   // Pay the barber and signal barber appropriately
   money_paid_[barbId] = true;
   pthread_cond_signal(&(cond_barber_paid_[barbId]));
   print(false, custId, "says good-bye to barber [" + int2string(barbId) + "]");
   pthread_mutex_unlock(&mutex_);
}

//---------------------------------------helloCustomer----------------------------------------------------------------------------------------------------------------
//Barber is accepting customers, requires barbId for parameter
void Shop::helloCustomer(int barbId)
{
   pthread_mutex_lock(&mutex_);

   // If no customers than barber can sleep
   if (waiting_chairs_.empty() && customer_in_barb_chair_[barbId] == 0)
   {
      print(true, barbId, "sleeps because of no customers.");
      pthread_cond_wait(&(cond_barber_sleeping_[barbId]), &mutex_);
   }

   if (customer_in_barb_chair_[barbId] == 0) // if no customer in chair, can sleep
   {
      pthread_cond_wait(&(cond_barber_sleeping_[barbId]), &mutex_);
   }
   print(true, barbId, "starts a hair-cut service for customer[" + int2string(customer_in_barb_chair_[barbId]) + "]");
   pthread_mutex_unlock(&mutex_);
}

//-------------------------------------byeCustomer-----------------------------------------------------------------------------------------------
//Barber is done with customer, requires barbId for parameter
void Shop::byeCustomer(int barbId)
{
   int nextCust = -1;
   pthread_mutex_lock(&mutex_);

   // Hair Cut-Service is done so signal customer and wait for payment
   in_service_[barbId] = false;
   print(true, barbId, "says he's done with a hair-cut service for customer [" + int2string(customer_in_barb_chair_[barbId]) + "]");
   money_paid_[barbId] = false;
   pthread_cond_signal(&(cond_customer_served_[customer_in_barb_chair_[barbId]]));
   while (money_paid_[barbId] == false)
   {
      pthread_cond_wait(&(cond_barber_paid_[barbId]), &mutex_);
   }

   // Now barber's chair is empty
   customer_in_barb_chair_[barbId] = 0;
   ready_barbers.push(barbId); //barber is put on ready queue
   print(true, barbId, "calls in another customer");
   if (count > 0)  //if barber needs to take from the waiting queue
   {
      nextCust = barbers_next_cust.front();
      pthread_cond_signal(&cond_customers_waiting_[nextCust]);
      barbers_next_cust.pop();
      count--;
   }

   pthread_mutex_unlock(&mutex_); 
}

