//Driver file

//This driver tests the sleeping-barbers problem.  It creates the shop, the barbers, and the clients.

//Assumptions and Implementation
// --Arguments will be in order: number of barbers, number of chairs, number of customers, barber's service time
// --Will output # of customers that did not get service at the end

//-----------------------------------------------------------------Driver---------------------------------------------------------------------------------------------------------------
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include "Shop.h"
using namespace std;

void *barber(void *);
void *customer(void *);

//---------------------------------------------------------------ThreadParam---------------------------------------------------------------------------------------------------------------
// This class is used as a way to pass more than one argument to a thread.
class ThreadParam
{
public:
    ThreadParam(Shop *shop, int barberId, int custId, int service_time) : shop(shop), barberId(barberId), custId(custId), service_time(service_time){};
    Shop *shop; 
    int barberId;
    int custId;
    int service_time; // barber service time
};

//-------------------------------------------------------------------main----------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // Read arguments from command line
    if (argc != 5)
    {
        cout << "Usage: num_barber num_chairs num_customers service_time" << endl;
        return -1;
    }
    int num_barbers = atoi(argv[1]);
    int num_chairs = atoi(argv[2]);
    int num_customers = atoi(argv[3]);
    int service_time = atoi(argv[4]);

    if(num_customers > maxProgrammedNumCust - 1) {
        cout << "Please enter number of customers less than " << maxProgrammedNumCust << " or adjust maxProgrammedNumCust in Shop.h" << endl;
        return -1;
    }

    pthread_t barber_thread[num_barbers]; 
    pthread_t customer_threads[num_customers];
    Shop shop(num_chairs, num_barbers); //creating shop

    // spawns the number of barber threads,
    for (int i = 0; i < num_barbers; i++)
    {
        int barbIdC = i;
        ThreadParam *barber_param = new ThreadParam(&shop, barbIdC, 0, service_time);
        pthread_create(&barber_thread[i], NULL, barber, barber_param);
    }

    // loops spawning number of customers
    for (int i = 0; i < num_customers; i++)
    {
        usleep(rand() % 1000);
        int custIdC = i + 1; //because customers start from 1
        ThreadParam *customer_param = new ThreadParam(&shop, 0, custIdC, 0);
        pthread_create(&customer_threads[i], NULL, customer, customer_param);
    }

    // Wait for customers to finish and cancel barber
    for (int i = 0; i < num_customers; i++)
    {
        pthread_join(customer_threads[i], NULL);
    }
    for (int i = 0; i < num_barbers; i++)
    {
        pthread_cancel(barber_thread[i]);
    }

    cout << "# customers who didn't receive a service = " << shop.get_cust_drops() << endl;
    return 0;
}

//------------------------------------------------------------*barber-------------------------------------------------------------------------------------------
//Each barber thread is implemented as follows
//Takes *arg parameter
void *barber(void *arg)
{
    ThreadParam *barber_param = (ThreadParam *)arg;
    Shop &shop = *barber_param->shop;
    int service_time = barber_param->service_time;
    int barbId = barber_param->barberId;
    delete barber_param;

    //loops the barber through accepting customers, servicing them, and letting them go
    while (true)
    {
        shop.helloCustomer(barbId);
        usleep(service_time);
        shop.byeCustomer(barbId);
    }
    return nullptr;
}

//--------------------------------------------------------*customer-----------------------------------------------------------------------------------------------
//Each customer thread is implemented as follows
//Takes *arg parameter
void *customer(void *arg)
{
    ThreadParam *customer_param = (ThreadParam *)arg;
    Shop &shop = *customer_param->shop;
    int custId = customer_param->custId;
    delete customer_param;

    int barberId = -1;
    barberId = shop.visitShop(custId);

    if (barberId != -1) //if they are being serviced, then they will proceed to make motions to leave the shop
    {
        shop.leaveShop(custId, barberId);
    }
    return nullptr;
}
