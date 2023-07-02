//Header file for Shop Class

//Assumptions and Implementation
// --Programmed for 399 customers (as Customers start from 1), can change this accordingly in maxProgrammedNumCust
// --Default number of chairs = 3, Default number of barbers = 1

//---------------------------Shop.h-------------------------------------------------------------------------------------------

#ifndef SHOP_H
#define SHOP_H
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
using namespace std;

#define kDefaultNumChairs 3
#define kDefaultNumBarbers 1
#define maxProgrammedNumCust 400 //if number of customers will be more than 399, need to change accordingly

class Shop
{
public:
   //Constructor
   Shop(int num_chairs, int num_barbers) : max_waiting_cust_((num_chairs > 0) ? num_chairs : kDefaultNumChairs),
                                                         cust_drops_(0), count(0), max_barber_number((num_barbers > 0) ? num_barbers : kDefaultNumBarbers)                                       
   {
      init();
   };

   //Default constructor
   Shop() : max_waiting_cust_(kDefaultNumChairs),
            cust_drops_(0), count(0), max_barber_number(kDefaultNumBarbers)
   {
      init();
   };

   //Destructor
   ~Shop();
   int visitShop(int custId); // return barber id or -1 if not served
   void leaveShop(int custId, int barbId);
   void helloCustomer(int barbId);
   void byeCustomer(int barbId);
   int get_cust_drops() const; //returns number of customer drops

private:
   const int max_waiting_cust_; // the max number of threads that can wait
  int max_barber_number; //number of barbers
   int *customer_in_barb_chair_; //array of keeping track of which cust is in which barb's chair
   bool *in_service_; // array of barbers in service
   bool *money_paid_;// array of barbers paid
   queue<int> waiting_chairs_; // who is in waiting room
   queue<int> barbers_next_cust; // next customer on waiting queue
   queue<int> ready_barbers; // available barbers
   int cust_drops_; // number of customers who left because of no seats
   
   int count; //counter that barber checks to see if they need to take from waiting queue

   // To coordinate threads
   pthread_mutex_t mutex_;
   pthread_cond_t *cond_customers_waiting_;
   pthread_cond_t *cond_customer_served_;
   pthread_cond_t *cond_barber_paid_;
   pthread_cond_t *cond_barber_sleeping_;

   // Helper methods
   bool barbersAllBusy(); 
   void init();
   string int2string(int i);
   void print(bool isBarber, int person, string message);
};
#endif
