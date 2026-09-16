
#define _GLOBAL_VARIABLES_H
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <iomanip>
#include <sstream>
#include <math.h>
using namespace std;

#define MAXVALUE 99999999
#define MININT -2147483648
#define MAXINT 2147483647
#define PRECISION 1.0e-8
#define MUTATION_INI_POP
#define SDF_MAX(a,b) ((a)>(b)?(a):(b))

char instanceName[] = "./CDP_Instances/CDP_Instances_b02/MDG-b_10_n500_b02_m50.txt";
int seed = 1;

typedef struct instance_data    
{
    int num_v;					//number of vertices
    double capacity;			//capacity constraint
    double *weight;				//weight of each vertex
    double **distance;			//distance between each pair of vertices
}instance_data;
instance_data Ins;		//instance data

//typedef struct{
//    double cost;				//the objective function value
//    double weight;				//the total weight of the solution
//    int num_sel;				//the number of selected vertices of the solution
//    int	*ss;					//representation of the solution, ss[i] = 1 indicates that vertex i is selected, 0 otherwise
//} Solution;
//Solution Sol_best;


typedef struct neighborhood{
    int  type;
    int  v;  //add
    int  g;  //drop
    int  x;  //swap_add
    int  y;  //swap_drop
}neighborhood;
neighborhood *Neighbors;

int L = pow(10, 6);    //L是禁忌表长，设置为10^8
int *W1, *W2, *W3;
int *H1, *H2, *H3;

double *Min_dis;
double *Sec_dis;
int *Num_min_dis;
int *Vec_min_dis;
double Max_min_dis;
double Sec_min_dis;

int Num_greedy_ini = 1000;

int Tabu_tenure = 30;
int *Tabu_list;
int tabu_depth = 3000;
int Non_imp_iters = 3000;

int Tabu_depth = 3000;
double Pert_str = 0.1;				//perturbation strength μ
//for memetic search
int Pop_size = 10;				//population size
int **Pop_sol;
double *Pop_obj;

double TWeight = 0;
double TProfit = 0;
int num_sel = 0;
int *Cur_sol;
int *Local_best_sol;
int *Best_sol_one_run;
int *Global_best_sol;
double Best_sol_one_run_obj;


double Time_limit = 200000.0;
double Start_time;
double Run_time;
int Max_generations = 10;


//#define DEBUG_READ_INSTANCE
#define MAXCAN 200
#define MAXNUM 999999999
#define INFEA -1



//read instance file
void read_instance(const char *instance_name)
{
    ifstream fin;
    fin.open(instance_name);
    if (fin.fail())
    {
        cout << "### error open, instance_name " << instance_name << endl;
        exit(-1);
    }

    fin >> Ins.num_v;
    fin >> Ins.capacity;
    Ins.weight = new double[Ins.num_v];

    int count_v = 0;
    while (count_v < Ins.num_v)
        fin >> Ins.weight[count_v++];
    Ins.distance = new double*[Ins.num_v];
    for (int i = 0; i < Ins.num_v; i++)
        Ins.distance[i] = new double[Ins.num_v];

    int line_count = 0;
    while (!fin.eof() && line_count < Ins.num_v)
    {
        count_v = 0;
        while (count_v < Ins.num_v)
            fin >> Ins.distance[line_count][count_v++];
        line_count++;
    }
    if (line_count != Ins.num_v)
    {
        cout << "### Error line_count != Ins.num_v, Ins.num_v=" << Ins.num_v
             << ", line_count=" << line_count << endl;
        exit(-1);
    }
    cout << "finished read, running CDP" << endl;
    cout << "Instance_name=" << instance_name << ", Num_v=" << Ins.num_v
         << ", capacity=" << Ins.capacity << endl << endl;

    fin.close();

#ifdef DEBUG_READ_INSTANCE
    cout << "num_v=" << Ins.num_v << endl;
    cout << "capacity=" << Ins.capacity << endl;
    cout << "weight=" << endl;
    for (int i = 0; i < Ins.num_v; i++)
        cout << Ins.weight[i] << " ";
    cout << endl << "distance=" << endl;;
    for (int i = 0; i < Ins.num_v; i++)
    {
        for (int j = 0; j < Ins.num_v; j++)
            cout << Ins.distance[i][j] << " ";
        cout << endl;
    }
#endif
}

//compute the objective of a solution,
//time complexity: O(n * |M|) where |M| is the number of vertices selected to the current solution
double compute_obj(int *sol)
{
    double obj_min = MAXVALUE;
    for (int i = 0; i < Ins.num_v; i++)
    {
        if (sol[i] == 1)
        {
            for (int j = i + 1; j < Ins.num_v; j++)
                if (sol[j] == 1)
                    if (Ins.distance[i][j] < obj_min - PRECISION)
                        obj_min = Ins.distance[i][j];
        }
    }
    return obj_min;
}

//copy solution_data structure
void copy_solution(int *source, int *des)
{
    memcpy(des, source, sizeof(int) * Ins.num_v);
//    des.num_sel = source.num_sel;
//    des.cost = source.cost;
//    des.weight = source.weight;
}

//initialize the global variables: Min_dis, Sec_dis, Num_min_dis, Vec_min_dis, time complexity: O(n * |M|)
void initialize_sup_arrays(int *sol)
{
    Max_min_dis = -MAXVALUE;
    Sec_min_dis = -MAXVALUE;
    for (int i = 0; i < Ins.num_v; i++)
    {
        Min_dis[i] = MAXVALUE;
        Sec_dis[i] = MAXVALUE;
        Num_min_dis[i] = 0;
        Vec_min_dis[i] = -1;
    }

    for (int i = 0; i < Ins.num_v; i++)
    {
        double dis_min = MAXVALUE;
        int select = -1;
        for (int j = 0; j < Ins.num_v; j++)
        {
            if (sol[j] && i != j)
            {
                if (Ins.distance[i][j] < dis_min - PRECISION)
                {
                    dis_min = Ins.distance[i][j];
                    select = j;
                    Num_min_dis[i] = 1;
                }
                else if (fabs(Ins.distance[i][j] - dis_min) <= PRECISION)
                    Num_min_dis[i]++;
            }
        }
        Min_dis[i] = dis_min;
        Vec_min_dis[i] = select; //only restore the first encountered vertex with dis_min

        //calculate Max_min_dis
        if (sol[i] == 0 && Min_dis[i] > Max_min_dis + PRECISION)
            Max_min_dis = Min_dis[i];
    }

    for (int i = 0; i < Ins.num_v; i++)
    {
        double dis_second_min = MAXVALUE;
//		int select = -1;
        for (int j = 0; j < Ins.num_v; j++)
        {
            if (sol[j] == 1 && i != j && j != Vec_min_dis[i] && Ins.distance[i][j] < dis_second_min - PRECISION)
            {
                dis_second_min = Ins.distance[i][j];
//				select = j;
            }
        }
        Sec_dis[i] = dis_second_min;

        //calculate Sec_min_dis
        if (sol[i] == 0 && Min_dis[i] > Sec_min_dis + PRECISION && fabs(Min_dis[i] - Max_min_dis) > PRECISION)
            Sec_min_dis = Min_dis[i];
    }
}

//generate a feasible initial solution in a greedy manner, time complexity: O(n * |M|)
void greedy_construct_initialsol(int *sol)
{
    memset(sol, 0, sizeof(int) * Ins.num_v);
//    for(int i = 0; i < Ins.num_v; i++)
//        sol.ss[i] = 0;
    num_sel = 0;
    TWeight = 0;
    //choose a start point
    TProfit = 0;
    int select = rand() % Ins.num_v;
//    sol[select] = 1;
//    TWeight += Ins.weight[select];
//    num_sel++;

    for(int i = 0; i < int(Ins.num_v / 25); i++)
    {
        select = rand() % Ins.num_v;
        if(sol[select] == 1)
        {
            i--;
            continue;
        }
        else if(sol[select] == 0)
        {
            sol[select] = 1;
            TWeight += Ins.weight[select];
            num_sel++;
        }
    }

    initialize_sup_arrays(sol);

    //greedy expansion
    while (TWeight < Ins.capacity - PRECISION)
    {
        int greedy_best_array[10000];    //数组的数太大了
        int best_len = 0;
        for (int i = 0; i < Ins.num_v; i++)
            if (sol[i] == 0 && fabs(Min_dis[i] - Max_min_dis) <= PRECISION)
                greedy_best_array[best_len++] = i;

        if (best_len > 0)
        {
            select = greedy_best_array[rand() % best_len];
            sol[select] = 1;
            num_sel++;
            TWeight += Ins.weight[select];
            initialize_sup_arrays(sol);
        }
    }
    TProfit = compute_obj(sol);
    //	verify_sol(sol); //verify solution

    //update global solution
//    if (sol.cost > Sol_best.cost + PRECISION)
//    {
//        copy_solution(sol, Sol_best);
        Run_time = (double) (clock() - Start_time) / CLOCKS_PER_SEC;
//    }
}

//compute move gain for f(M) for each candidate move
double compute_flip_move_gain(int node, int *sol)
{
    double move_gain = 0;
    /* ADD: V\M --> M,
     * if Min_dis[node] >= sol.cost, then move_gain = 0
     * otherwise, move_gain is decreased
     */
    if (sol[node] == 0)
    {
        if (Min_dis[node] > TProfit + PRECISION || fabs(Min_dis[node] - TProfit) <= PRECISION)
            move_gain = 0;
        else
            move_gain = Min_dis[node] - TProfit;
    }
        /* DROP: M --> V\M, only two cases:
         * case 1: if Min_dis[node] > 0, then move_gain = 0
         * case 2: if Min_dis[node] = 0, we evaluate for how many nodes, its Min_dis[] = 0
         */
    else
    {
        if (Min_dis[node] > TProfit + PRECISION)
            move_gain = 0;
        else if (fabs(Min_dis[node] - TProfit) <= PRECISION)
        {
            double cost_min = MAXVALUE;
            /* to evaluate if there are more than one node their Min_dis[] = sol.cost
             * if only one node it Min_dis[] = sol.cost, then cost_min = Sec_dis[i] (changed)
             * if more than one node its Min_dis[] = sol.cost, then cost_min = Min_dis[i] (= sol.cost) (unchanged)
             */
            for (int i = 0; i < Ins.num_v; i++)
            {
                if (sol[i] == 1 && i != node)
                {
                    double poten_tmp = Min_dis[i];
                    if (Vec_min_dis[i] == node)
                        poten_tmp = Sec_dis[i];
                    if (poten_tmp < cost_min - PRECISION)
                        cost_min = poten_tmp;
                }
            }
            move_gain = cost_min - TProfit;
        }
    }
    return move_gain;
}

double compute_swap_move_gain(int node_x, int node_y, int *sol)   //compute_swap_move_gain(add, drop, sol)
{
    double move_gain = 0;

    if (Min_dis[node_x] > TProfit + PRECISION || fabs(Min_dis[node_x] - TProfit) <= PRECISION)
    {
        if (Min_dis[node_y] > TProfit + PRECISION)
            move_gain = 0;
        else if (fabs(Min_dis[node_y] - TProfit) <= PRECISION)
        {
            double cost_min = MAXVALUE;
            for (int i = 0; i < Ins.num_v; i++)
            {
                if (sol[i] == 1 && i != node_y)
                {
                    double poten_tmp = Min_dis[i];
                    if (Vec_min_dis[i] == node_y)
                        poten_tmp = Sec_dis[i];
                    if (poten_tmp < cost_min - PRECISION)
                        cost_min = poten_tmp;
                }
            }

            if (Min_dis[node_x] < cost_min - PRECISION)
            {
                if (fabs(Ins.distance[node_x][node_y] - Min_dis[node_x]) > PRECISION)
                    move_gain = Min_dis[node_x] - TProfit;
                else
                {
                    if (Num_min_dis[node_x] == 1)
                    {
                        if (Sec_dis[node_x] < cost_min - PRECISION)
                            move_gain = Sec_dis[node_x] - TProfit;
                        else
                            move_gain = cost_min - TProfit;
                    }
                    else
                        move_gain = Min_dis[node_x] - TProfit;
                }
            }
            else
                move_gain = cost_min - TProfit;
        }
    }
    else
    {
        if (Min_dis[node_y] > TProfit + PRECISION)
        {
            if (fabs(Ins.distance[node_x][node_y] - Min_dis[node_x]) > PRECISION)
                move_gain = Min_dis[node_x] - TProfit;
            else
            {
                if (Num_min_dis[node_x] == 1)
                    move_gain = 0;
                else
                    move_gain = Min_dis[node_x] - TProfit;
            }
        }
        else if (fabs(Min_dis[node_y] - TProfit) <= PRECISION)
        {
            double cost_min = MAXVALUE;
            for (int i = 0; i < Ins.num_v; i++)
            {
                if (sol[i] == 1 && i != node_y)
                {
                    double poten_tmp = Min_dis[i];
                    if (Vec_min_dis[i] == node_y)
                        poten_tmp = Sec_dis[i];
                    if (poten_tmp < cost_min - PRECISION)
                        cost_min = poten_tmp;
                }
            }

            if (Min_dis[node_x] < cost_min - PRECISION)
            {
                if (fabs(Ins.distance[node_x][node_y] - Min_dis[node_x]) > PRECISION)
                    move_gain = Min_dis[node_x] - TProfit;
                else
                {
                    if (Num_min_dis[node_x] == 1)
                    {
                        if (Sec_dis[node_x] < cost_min - PRECISION)
                            move_gain = Sec_dis[node_x] - TProfit;
                        else
                            move_gain = cost_min - TProfit;
                    }
                    else
                        move_gain = Min_dis[node_x] - TProfit;
                }
            }
            else
                move_gain = cost_min - TProfit;
        }
    }
    return move_gain;
}


void verify_sol(int *sol)
{
    double weight = 0;
    double cost = MAXVALUE;
    int tmp_num_sel = 0;
    for (int i = 0; i < Ins.num_v; i++)
    {
        if (sol[i])
        {
            tmp_num_sel++;
            weight += Ins.weight[i];
        }
    }
    cost = compute_obj(sol);

    //verify
    if (weight < Ins.capacity - PRECISION)
    {
        cout << "sol's weight < Capacity, an infeasible solution obtained!!!!!!" << endl;
        exit(-1);
    }
    if (fabs(TWeight - weight) > PRECISION)
    {
        cout << "sol.weight=" << TWeight << ", weight=" << weight << endl;
        cout << "sol.weight != weight, an infeasible solution obtained!!!!!!" << endl;
        exit(-1);
    }
    if (fabs(TProfit - cost) > PRECISION)
    {
        cout << "sol.cost=" << TProfit << ", cost=" << cost << endl;
        cout << "sol.cost != cost, an infeasible solution obtained!!!!!!" << endl;
        exit(-1);
    }
    if (num_sel != tmp_num_sel)
    {
        cout << "sol.num_sel != num_sel, an infeasible solution obtained!!!!!!" << endl;
        exit(-1);
    }
}

void update_best_sol_one_run(double f_sol, int *sol)
{
    Best_sol_one_run_obj = f_sol;
    for (int j = 0; j < Ins.num_v; j++)
        Best_sol_one_run[j] = sol[j];
    Run_time = 1.0*(clock() - Start_time) / CLOCKS_PER_SEC;
}

bool is_same_solution(int *sol_1, int *sol_2)
{
    bool is_exist = true;
    for (int j = 0; j < Ins.num_v; j++)
    {
        if (sol_1[j] != sol_2[j])
        {
            is_exist = false;
            break;
        }
    }
    return is_exist;
}


//for the perturbation
void build_neighbors()
{
    int count = 0;
    //int numbers = N*(N - 1) / 2 + N*K;

    //add neighborhood
    for (int i = 0; i < Ins.num_v; i++)
    {
        Neighbors[count].type = 1;
        Neighbors[count].v = i;
        count++;
    }

    //drop neighborhood
    for (int i = 0; i < Ins.num_v; i++)
    {
        Neighbors[count].type = 2;
        Neighbors[count].g = i;
        count++;
    }

    //swap neighborhood
    for (int i = 0; i < Ins.num_v; i++)
    {
        for (int j = i + 1; j < Ins.num_v; j++)
        {
            Neighbors[count].type = 3;
            Neighbors[count].x = i;
            Neighbors[count].y = j;
            count++;
        }
    }
}

void random_perturbation(int *sol, int pert_len)
{
    int count = 0;
    int number_neighbors = Ins.num_v * (Ins.num_v - 1) / 2 + 2 * Ins.num_v ;
    do
    {
        int cur_index = rand() % number_neighbors;
        if (Neighbors[cur_index].type == 1)   //add
        {
            int v = Neighbors[cur_index].v;
            if (sol[v] == 0)
            {
                TProfit += compute_flip_move_gain(v, sol);
                num_sel++;
                TWeight += Ins.weight[v];
                sol[v] = 1;
                count++;
            }
        }
        else if (Neighbors[cur_index].type == 2)    //drop
        {
            int g = Neighbors[cur_index].g;
            if (sol[g] == 1 && ((TWeight - Ins.weight[g]) > Ins.capacity + PRECISION
                || fabs(TWeight - Ins.weight[g] - Ins.capacity) <= PRECISION))
            {
                TProfit += compute_flip_move_gain(g, sol);
                num_sel--;
                TWeight -= Ins.weight[g];
                sol[g] = 0;
                count++;
            }

        }
        else if (Neighbors[cur_index].type == 3)    //swap
        {
            int x = Neighbors[cur_index].x;
            int y = Neighbors[cur_index].y;
            if (sol[x] == 0 && sol[y] == 1 && (TWeight + Ins.weight[x] - Ins.weight[y]) > Ins.capacity - PRECISION)
            {
                TProfit += compute_swap_move_gain(x, y, sol);
                TWeight = TWeight + Ins.weight[x] - Ins.weight[y];
                sol[x] = 1;
                sol[y] = 0;
                count++;
            }
            else if (sol[x] == 1 && sol[y] == 0 && (TWeight + Ins.weight[y] - Ins.weight[x]) > Ins.capacity - PRECISION)
            {
                TProfit += compute_swap_move_gain(y, x, sol);
                TWeight = TWeight + Ins.weight[y] - Ins.weight[x];
                sol[y] = 1;
                sol[x] = 0;
                count++;
            }
        }
    } while (count < pert_len);
}

//allocate memory for some global variables
void allocate_memory()
{

    Min_dis = new double[Ins.num_v];
    Sec_dis = new double[Ins.num_v];
    Num_min_dis = new int[Ins.num_v];
    Vec_min_dis = new int[Ins.num_v];
    int number_neighbors = Ins.num_v * (Ins.num_v - 1) / 2 + Ins.num_v * 2;
    Neighbors = new neighborhood [number_neighbors];

    Pop_sol = new int*[Pop_size];
    for (int p = 0; p < Pop_size; p++)
        Pop_sol[p] = new int[Ins.num_v];
    Pop_obj = new double [Pop_size];
    Cur_sol = new int [Ins.num_v];
    Global_best_sol = new int[Ins.num_v];
    Best_sol_one_run = new int[Ins.num_v];
    Local_best_sol = new int[Ins.num_v];

    Tabu_list = new int[Ins.num_v];

    W1 = new int[Ins.num_v];
    W2 = new int[Ins.num_v];
    W3 = new int[Ins.num_v];
    H1 = new int[L];
    H2 = new int[L];
    H3 = new int[L];
}

void free_memory()
{
    delete[] Ins.weight;
    for (int i = 0; i < Ins.num_v; i++)
        delete[] Ins.distance[i];
    delete[] Ins.distance;

    delete[]Neighbors; Neighbors = NULL;

    delete[] Min_dis;
    delete[] Sec_dis;
    delete[] Num_min_dis;
    delete[] Vec_min_dis;

    for (int p = 0; p < Pop_size; p++)
    {
        delete[]Pop_sol[p]; Pop_sol[p] = NULL;
    }
    delete[]Pop_sol; Pop_sol = NULL;
    delete[]Pop_obj; Pop_obj = NULL;
    delete[]Cur_sol; Cur_sol = NULL;
    delete[]Global_best_sol; Global_best_sol = NULL;
    delete[]Best_sol_one_run; Best_sol_one_run = NULL;
    delete[]Local_best_sol; Local_best_sol = NULL;

    delete []Tabu_list; Tabu_list = NULL;

    delete[] W1;
    delete[] W2;
    delete[] W3;
    delete[] H1;
    delete[] H2;
    delete[] H3;
}

double descent_local_search(int *sol, double &f_sol)
{
    int *rand_item = new int[Ins.num_v];
    int *flag_item = new int[Ins.num_v];
    int flag_improve = 1;
    //while (flag_improve && 1.0*(clock() - Start_time) / CLOCKS_PER_SEC < Time_limit)
    while (flag_improve)
    {
        flag_improve = 0;
        for (int j = 0; j < Ins.num_v; j++)
            flag_item[j] = 0;
        int len1 = 0;
        while (len1 < Ins.num_v)
        {
            int r_item = rand() % Ins.num_v;
            if (flag_item[r_item] == 0)
            {
                rand_item[len1++] = r_item;     //rand_item是打乱顺序的所有item
                flag_item[r_item] = 1;
            }
        }

        //for add operator, 后续add可以去掉
//        for (int j = 0; j < Ins.num_v; j++)
//        {
//            int item = rand_item[j];
//            if(sol[item] == 0)
//            {
//                double delta = compute_flip_move_gain(item, sol);
//                if(delta == 0)
//                {
//                    TProfit += delta;
//                    num_sel++;
//                    TWeight += Ins.weight[item];
//                    sol[item] = 0;
//                    flag_improve = 1;
//                    f_sol = TProfit;
//                    if (f_sol > Best_sol_one_run_obj + PRECISION)
//                        update_best_sol_one_run(f_sol, sol);
//                }
//            }
//        }

        //for drop operator
//        for (int j = 0; j < Ins.num_v; j++)
//        {
//            int item = rand_item[j];
//            if(sol[item] == 1 && ((TWeight - Ins.weight[item]) > Ins.capacity + PRECISION
//                || fabs(TWeight - Ins.weight[item] - Ins.capacity) <= PRECISION))
//            {
//                double delta = compute_flip_move_gain(item, sol);;
//                if(delta > 0)
//                {
//                    TProfit += delta;
//                    num_sel--;
//                    TWeight -= Ins.weight[item];
//                    sol[item] = 0;
//                    flag_improve = 1;
//                    f_sol = TProfit;
//                    if (f_sol > Best_sol_one_run_obj + PRECISION)
//                        update_best_sol_one_run(f_sol, sol);
//                }
//            }
//        }

//        for (int j = 0; j < numItem; j++)
//            cout<<sol[j]<<" ";
//        cout<<f_sol;

        //for swap operator
        for (int j1 = 0; j1 < Ins.num_v; j1++)
        {
            int item_j1 = rand_item[j1];
            for (int j2 = j1 + 1; j2 < Ins.num_v; j2++)
            {
                int item_j2 = rand_item[j2];
                if (sol[item_j1] == 0 && sol[item_j2] == 1 &&
                    (TWeight + Ins.weight[item_j1] - Ins.weight[item_j2]) > Ins.capacity - PRECISION)
                {
                    double delta = compute_swap_move_gain(item_j1, item_j2, sol);
                    if (delta > 0 + PRECISION)
                    {
                        TProfit += delta;
                        TWeight = TWeight + Ins.weight[item_j1] - Ins.weight[item_j2];
                        sol[item_j1] = 1;
                        sol[item_j2] = 0;
                        flag_improve = 1;
                        f_sol = TProfit;
                        if (f_sol > Best_sol_one_run_obj + PRECISION)
                            update_best_sol_one_run(f_sol, sol);
                    }
                }
                else if(sol[item_j1] == 1 && sol[item_j2] == 0 &&
                        (TWeight + Ins.weight[item_j2] - Ins.weight[item_j1]) > Ins.capacity - PRECISION)
                {
                    double delta = compute_swap_move_gain(item_j2, item_j1, sol);
                    if (delta > 0 + PRECISION)
                    {
                        TProfit += delta;
                        TWeight = TWeight + Ins.weight[item_j2] - Ins.weight[item_j1];
                        sol[item_j1] = 0;
                        sol[item_j2] = 1;
                        flag_improve = 1;
                        f_sol = TProfit;
                        if (f_sol > Best_sol_one_run_obj + PRECISION)
                            update_best_sol_one_run(f_sol, sol);
                    }
                }
            }
        }
    }
    delete[]rand_item; rand_item = NULL;
    delete[]flag_item; flag_item = NULL;
    return f_sol;
}

void build_data(int *sol, int *pop_sol)
{
    TProfit = 0;
    TWeight = 0;
    num_sel = 0;
    for(int i = 0; i < Ins.num_v; i++)
        sol[i] = pop_sol[i];

    for(int i = 0; i < Ins.num_v; i++)
        if(sol[i] == 1)
        {
            num_sel++;
            TWeight += Ins.weight[i];
        }
    TProfit = compute_obj(sol);
}


//initial the population
void initial_population()
{
    int *sol = new int[Ins.num_v];

    num_sel = 0;
    TWeight = 0;
    TProfit = 0;
    for(int i = 0; i < Ins.num_v; i++)
        sol[i] = 0;

    int **sol_tmp = new int *[Pop_size];            //Sp，包含p个可行解
    for (int p = 0; p < Pop_size; p++)
        sol_tmp[p] = new int[Ins.num_v];
    double *obj_tmp = new double[Pop_size];          //p个可行解中每个解的函数值
    for (int i = 0; i < Pop_size; i++)
        obj_tmp[i] = -MAXNUM;
    double time1 = clock();

    for (int i = 0; i < Num_greedy_ini; i++)
    {
        greedy_construct_initialsol(sol);

        double f_sol = TProfit;

//        cout<<" f_sol = "<<f_sol<<endl;

        int sel_pos = Pop_size;
        for (int k = 0; k < Pop_size; k++)
        {
            if (f_sol > obj_tmp[k] + PRECISION)
            {
                sel_pos = k;
                break;
            }
        }
        for (int j = Pop_size - 1; j > sel_pos; j--)    //后移一位，空出可插入的位置
        {
            obj_tmp[j] = obj_tmp[j - 1];
            memcpy(sol_tmp[j], sol_tmp[j - 1], sizeof(int) * Ins.num_v);
        }
        if (sel_pos != Pop_size)
        {
            obj_tmp[sel_pos] = f_sol;
            memcpy(sol_tmp[sel_pos], sol, sizeof(int) * Ins.num_v);
        }

    }
    double initial_time = 1.0 * (clock() - time1) / CLOCKS_PER_SEC;
    cout << "in initial_sol func, initial_time=" << initial_time << endl;

//    for(int i = 0; i < Pop_size; i++)
//    {
//        for(int j = 0; j < Ins.num_v; j++)
//            cout<<sol_tmp[i][j]<<" ";
//        cout<<endl;
//    }
//    for(int i = 0; i < Pop_size; i++)
//        cout<<obj_tmp[i]<<endl;


    double max_objective = 0;
    int best_sol = 0;
    for (int pop_len = 0; pop_len < Pop_size; pop_len++)
        //int pop_len = 0;
        //while (pop_len < Pop_size)
    {
        memcpy(Pop_sol[pop_len], sol_tmp[pop_len], sizeof(int) * Ins.num_v);
        //greedy_initial_sol(Pop_sol[pop_len]);
        build_data(sol, Pop_sol[pop_len]);

        double f_part = TProfit;
        Pop_obj[pop_len] = f_part;

        descent_local_search(sol, Pop_obj[pop_len]);

        for(int m = 0; m < Ins.num_v; m++)
            Pop_sol[pop_len][m] = sol[m];

//        multi_neighbor_tabu_search(Pop_sol[pop_len], Pop_obj[pop_len]);


        if(Pop_obj[pop_len] > max_objective)
        {
            max_objective = Pop_obj[pop_len];
            best_sol = pop_len;
        }

#ifdef MUTATION_INI_POP
        while (1)
        {
            bool is_exist = false;
            for (int i = 0; i < pop_len; i++)
            {
                is_exist = is_same_solution(Pop_sol[i], Pop_sol[pop_len]);

                if (is_exist)
                    break;
            }
            if (!is_exist)
                break;
            else //是same solution，需要扰动
            {
                build_data(sol, Pop_sol[pop_len]);
                random_perturbation(sol, int(Pert_str * Ins.num_v));
                for(int i = 0; i < Ins.num_v; i++)
                    Pop_sol[pop_len][i] = sol[i];
            }
            cout << "pop_len=" << pop_len << ", is_exist=" << is_exist << endl;
        }
#endif
        for(int i = 0; i < Ins.num_v; i++)
            cout <<Pop_sol[pop_len][i]<<" ";
        cout<<endl;
        Pop_obj[pop_len] = compute_obj(Pop_sol[pop_len]);
        cout<<"Pop_obj = "<<Pop_obj[pop_len]<<endl;
        cout<<endl;
    }

    double best_obj = 0;
    int index = 0;
    for(int i = 0; i <Pop_size; i++)
        if(best_obj < Pop_obj[i])
        {
            best_obj = Pop_obj[i];
            index = i;
        }
    update_best_sol_one_run(best_obj, Pop_sol[index]);
//    for (int i = 0; i < Pop_size; i++)
//       verify_sol proof(Pop_sol[i], Pop_obj[i]);
    cout << "after initial_population func, " << ", Best_sol_one_run_obj=" << Best_sol_one_run_obj << endl;

//    for(int i = 0; i < Ins.num_v; i++)
//        Cur_sol.ss[i] = Pop_sol[best_sol][i];

    for (int i = 0; i < Pop_size; i++)
    {
        delete[]sol_tmp[i]; sol_tmp[i] = NULL;
    }
    delete[]sol_tmp; sol_tmp = NULL;
    delete[]obj_tmp; obj_tmp = NULL;
    delete[]sol; sol = NULL;

}

//update the population, update the worst solution in terms of the objective value
void update_population(int *offspring, double f_off)
{
    double obj_worst = MAXNUM;
    int index_worst = -1;
    for (int i = 0; i < Pop_size; i++)
    {
        if (Pop_obj[i] < obj_worst - PRECISION)
        {
            obj_worst = Pop_obj[i];
            index_worst = i;
        }
    }
    if (f_off > obj_worst + PRECISION)
    {
        bool is_exist = is_same_solution(Pop_sol[index_worst], offspring);
        if (!is_exist)
        {
            memcpy(Pop_sol[index_worst], offspring, sizeof(int) * Ins.num_v);
            Pop_obj[index_worst] = f_off;
        }
    }
}
//只进行了预先计算权重向量Wv的随机洗牌操作,执行完之后的Wv是打乱顺序的
void initial_hash()
{
    int rand_num, temp_num;

    for(int i = 0; i < Ins.num_v; i++)    //Wi is a pre-computed weight γ的取值为1.2，1.6，2.0
    {
        W1[i] = (int) pow(i+1, 1.2);  //γ的值到时候可以做一个实验，换一些数值做一个表格
        W2[i] = (int) pow(i+1, 1.6);
        W3[i] = (int) pow(i+1, 2.0);
    }

    for(int i = 0; i < Ins.num_v; i++)
    {
        while(1)          //接下来的三个while是生成三个随机数，对预先计算的权重向量Wv中的顺序进行随机洗牌
        {
            rand_num = rand() % Ins.num_v;
            if(rand_num != i)
                break;
        }
        temp_num = W1[i];        //交换i和随即生成的序号
        W1[i] = W1[rand_num];
        W1[rand_num] = temp_num;

        while(1)
        {
            rand_num = rand() % Ins.num_v;
            if(rand_num != i)
                break;
        }
        temp_num = W2[i];
        W2[i] = W2[rand_num];
        W2[rand_num] = temp_num;

        while(1)
        {
            rand_num = rand() % Ins.num_v;
            if(rand_num != i)
                break;
        }
        temp_num = W3[i];
        W3[i] = W3[rand_num];
        W3[rand_num] = temp_num;
    }

    W1[Ins.num_v] = 0;
    W2[Ins.num_v] = 0;
    W3[Ins.num_v] = 0;
}


//solution based tabu search: Add, Drop, and Swap move operators with hash function
double solution_based_tabu_search(int *sol, double &f_sol)
{
    //initial hash
    int hx1 = 0, hx2 = 0, hx3 = 0;
    int hx11, hx22, hx33;

    for (int i = 0; i < L; i++)
    {
        H1[i] = 0;
        H2[i] = 0;
        H3[i] = 0;
    }
    for (int i = 0; i < Ins.num_v; i++)
    {
        if(sol[i])
        {
            hx1 += W1[i];
            hx2 += W2[i];
            hx3 += W3[i];
        }
    }
    double add_delta_max,drop_delta_max,swap_delta_max;
    double tabu_add_delta_max,tabu_drop_delta_max,tabu_swap_delta_max;
    int add_arr[500],drop_arr[500],swap_arr1[500], swap_arr2[500];
    int tabu_add_arr[500],tabu_drop_arr[500],tabu_swap_arr1[500], tabu_swap_arr2[500];
    int add_len, drop_len, swap_len;
    int tabu_add_len, tabu_drop_len, tabu_swap_len;
    int non_improve = 0;

    int *loc_best_sol = new int[Ins.num_v];
    copy_solution(sol, loc_best_sol);
    double loc_best_sol_obj = TProfit;
    initialize_sup_arrays(sol);

    while (non_improve < Tabu_depth)
    {
        add_delta_max = -MAXVALUE;
        tabu_add_delta_max = -MAXVALUE;
        add_len = 0;
        tabu_add_len = 0;
        //1. Add
        for (int i = 0; i < Ins.num_v; i++)
        {
            if (sol[i] == 0)
            {
                double cur_gain = compute_flip_move_gain(i, sol);

                hx11 = hx1 + W1[i];
                hx22 = hx2 + W2[i];
                hx33 = hx3 + W3[i];

                if (H1[hx11 % L] == 1 && H2[hx22 % L] == 1 && H3[hx33 % L] == 1)
                {
                    if (cur_gain > tabu_add_delta_max + PRECISION)
                    {
                        tabu_add_delta_max = cur_gain;
                        tabu_add_len = 0;
                        tabu_add_arr[tabu_add_len++] = i;
                    }
                    else if (fabs(cur_gain - tabu_add_delta_max) <= PRECISION && tabu_add_len < 500)
                    {
                        tabu_add_arr[tabu_add_len++]=i;
                    }
                }
                else
                {
                    if (cur_gain > add_delta_max + PRECISION)
                    {
                        add_delta_max = cur_gain;
                        add_len = 0;
                        add_arr[add_len++] = i;
                    }
                    else if (fabs(cur_gain - add_delta_max) <= PRECISION && add_len < 500)
                    {
                        add_arr[add_len++] = i;
                    }
                }
            }
        }

        drop_delta_max=-MAXVALUE;
        tabu_drop_delta_max=-MAXVALUE;
        drop_len=0;
        tabu_drop_len=0;
        //2. Drop
        for (int i = 0; i < Ins.num_v; i++)
        {
            if (sol[i] == 1 && ((TWeight - Ins.weight[i]) > Ins.capacity + PRECISION
                                   || fabs(TWeight - Ins.weight[i] - Ins.capacity) <= PRECISION))
            {
                double cur_gain = compute_flip_move_gain(i, sol);

                hx11 = hx1 - W1[i];
                hx22 = hx2 - W2[i];
                hx33 = hx3 - W3[i];

                if (H1[hx11 % L] == 1 && H2[hx22 % L] == 1 && H3[hx33 % L] == 1)
                {
                    if (cur_gain > tabu_drop_delta_max + PRECISION)
                    {
                        tabu_drop_delta_max = cur_gain;
                        tabu_drop_len = 0;
                        tabu_drop_arr[tabu_drop_len++] = i;
                    }
                    else if (fabs(cur_gain - tabu_drop_delta_max) <= PRECISION && tabu_drop_len < 500)
                    {
                        tabu_drop_arr[tabu_drop_len++] = i;
                    }
                }
                else
                {
                    if (cur_gain > drop_delta_max + PRECISION)
                    {
                        drop_delta_max = cur_gain;
                        drop_len = 0;
                        drop_arr[drop_len++] = i;
                    }
                    else if (fabs(cur_gain - drop_delta_max) <= PRECISION && drop_len < 500)
                    {
                        drop_arr[drop_len++] = i;
                    }
                }
            }
        }

        swap_delta_max = -MAXVALUE;
        tabu_swap_delta_max = -MAXVALUE;
        swap_len = 0;
        tabu_swap_len = 0;
        //3. Swap
        int add_array_swap[500];
        int drop_array_swap[500];
        int add_len_swap = 0;
        int drop_len_swap = 0;
        for (int i = 0; i < Ins.num_v; i++)
        {
            //Find add nodes with max and sec Min_dis[i]
            if (sol[i] == 0 && (fabs(Min_dis[i] - Max_min_dis) <= PRECISION
                                   || fabs(Min_dis[i] - Sec_min_dis) <= PRECISION))
                add_array_swap[add_len_swap++] = i;
            //Find all drop nodes with a distance equal to sol.cost
            if (sol[i] == 1 && fabs(Min_dis[i] - TProfit) <= PRECISION)
                drop_array_swap[drop_len_swap++] = i;
        }
        for (int i = 0; i < add_len_swap; i++)
        {
            int nx = add_array_swap[i];
            for (int j = 0; j < drop_len_swap; j++)
            {
                int ny = drop_array_swap[j];
                if ((TWeight + Ins.weight[nx] - Ins.weight[ny]) < Ins.capacity - PRECISION)
                    continue;
                double cur_gain = compute_swap_move_gain(nx, ny, sol);

                //SWAP
                hx11 = hx1 + (W1[nx] - W1[ny]);
                hx22 = hx2 + (W2[nx] - W2[ny]);
                hx33 = hx3 + (W3[nx] - W3[ny]);

                if (H1[hx11 % L] == 1 && H2[hx22 % L] == 1 && H3[hx33 % L] == 1)
                {
                    if (cur_gain > tabu_swap_delta_max + PRECISION)
                    {
                        tabu_swap_delta_max = cur_gain;
                        tabu_swap_len = 0;
                        tabu_swap_arr1[tabu_swap_len] = ny;
                        tabu_swap_arr2[tabu_swap_len] = nx;
                        tabu_swap_len++;
                    }
                    else if (fabs(cur_gain - tabu_swap_delta_max) <= PRECISION && tabu_swap_len < 500)
                    {
                        tabu_swap_arr1[tabu_swap_len] = ny;
                        tabu_swap_arr2[tabu_swap_len] = nx;
                        tabu_swap_len++;
                    }
                }
                else
                {
                    if (cur_gain > swap_delta_max + PRECISION)
                    {
                        swap_delta_max = cur_gain;
                        swap_len = 0;
                        swap_arr1[swap_len] = ny;
                        swap_arr2[swap_len] = nx;
                        swap_len++;
                    }
                    else if (fabs(cur_gain - swap_delta_max) <= PRECISION && swap_len < 500)
                    {
                        swap_arr1[swap_len] = ny;
                        swap_arr2[swap_len] = nx;
                        swap_len++;
                    }
                }
            }
        }

        //select the best move from add, drop, swap neighborhoods
        int move_type = INFEA;
        double delta_maximum = -MAXVALUE;
        //cout<<"add_delta_max="<<add_delta_max<<", tabu_add_delta_max="<<tabu_add_delta_max<<", drop_delta_max="<<drop_delta_max<<", tabu_drop_delta_max"<<tabu_drop_delta_max
        //	<<", swap_delta_max="<<swap_delta_max<<", tabu_swap_delta_max="<<tabu_swap_delta_max<<endl;
        //cout<<" add_len="<<add_len<<", tabu_add_len="<<tabu_add_len<<", drop_len="<<drop_len<<", tabu_dorp_len="<<tabu_drop_len<<", swap_len=" <<swap_len<<", tabu_swap_len="<<tabu_swap_len<<endl;
        if (add_len > 0 && add_delta_max > delta_maximum)
        {
            move_type = 1;
            delta_maximum = add_delta_max;
        }
        if ((tabu_add_len > 0 && tabu_add_delta_max > delta_maximum && TProfit + tabu_add_delta_max > loc_best_sol_obj))
        {
            move_type = 2;
            delta_maximum = tabu_add_delta_max;
        }
        if (drop_len > 0 && drop_delta_max > delta_maximum)
        {
            move_type = 3;
            delta_maximum = drop_delta_max;
        }
        if ((tabu_drop_len > 0 && tabu_drop_delta_max > delta_maximum && TProfit + tabu_drop_delta_max > loc_best_sol_obj))
        {
            move_type = 4;
            delta_maximum = tabu_drop_delta_max;
        }
        if (swap_len > 0 && swap_delta_max > delta_maximum)
        {
            move_type = 5;
            delta_maximum = swap_delta_max;
        }
        if ((tabu_swap_len > 0 && tabu_swap_delta_max > delta_maximum && TProfit + tabu_swap_delta_max > loc_best_sol_obj))
        {
            move_type = 6;
            delta_maximum = tabu_swap_delta_max;
        }

        int item1 = INFEA, item2 = INFEA, item = INFEA;
        int rx = INFEA;
        switch(move_type)
        {
            case 1:
                rx = rand() % add_len;
                item = add_arr[rx];
                break;
            case 2:
                rx = rand() % tabu_add_len;
                item = tabu_add_arr[rx];
                break;
            case 3:
                rx = rand() % drop_len;
                item = drop_arr[rx];
                break;
            case 4:
                rx = rand() % tabu_drop_len;
                item = tabu_drop_arr[rx];
                break;
            case 5:
                rx = rand() % swap_len;
                item1 = swap_arr1[rx];
                item2 = swap_arr2[rx];
                break;
            case 6:
                rx = rand() % tabu_swap_len;
                item1 = tabu_swap_arr1[rx];    //item1 : drop
                item2 = tabu_swap_arr2[rx];    //item2 : add
                break;
        }

        if (move_type == 1 || move_type == 2)
        {
            sol[item] = 1;
            TWeight += Ins.weight[item];
            hx1 += W1[item];
            hx2 += W2[item];
            hx3 += W3[item];
            H1[hx1 % L] = 1;
            H2[hx2 % L] = 1;
            H3[hx3 % L] = 1;
        }
        else if (move_type == 3 || move_type == 4)
        {
            sol[item] = 0;
            TWeight -= Ins.weight[item];
            hx1 -= W1[item];
            hx2 -= W2[item];
            hx3 -= W3[item];
            H1[hx1 % L] = 1;
            H2[hx2 % L] = 1;
            H3[hx3 % L] = 1;
        }
        else if (move_type == 5 || move_type == 6)
        {
            sol[item1] = 0;
            sol[item2] = 1;
            TWeight = TWeight + Ins.weight[item2] - Ins.weight[item1];
            hx1 = hx1 + (W1[item2] - W1[item1]);
            hx2 = hx2 + (W2[item2] - W2[item1]);
            hx3 = hx3 + (W3[item2] - W3[item1]);
            H1[hx1 % L] = 1;
            H2[hx2 % L] = 1;
            H3[hx3 % L] = 1;
        }
        if (move_type != INFEA)
        {
            TProfit += delta_maximum;
            f_sol = TProfit;
            if (TProfit > loc_best_sol_obj)
            {
//                cout<<"TProfit = "<<TProfit<<", loc_best_sol_obj = "<<loc_best_sol_obj<<endl;
                non_improve = 0;
                copy_solution(sol, loc_best_sol);
                loc_best_sol_obj = TProfit;
            }
            else
                non_improve++;
        }
        if (item != -1 || item1 != -1 || item2 != -1)
        {
            initialize_sup_arrays(sol);
            //		verify_sol(sol); //verify solution
        }

        if (f_sol > Best_sol_one_run_obj + PRECISION)
            update_best_sol_one_run(loc_best_sol_obj, loc_best_sol);
    }
    copy_solution(loc_best_sol, sol);
    //update global solution
    f_sol = loc_best_sol_obj;
    build_data(sol, loc_best_sol);
    delete[] loc_best_sol; loc_best_sol = NULL;
    return f_sol;
    //free
}


//multi neighborhood tabu search
double multi_neighbor_tabu_search(int *sol, double &f_sol)
{
//	int add_delta,drop_delta,swap_delta;
//	int tabu_add_delta,tabu_drop_delta,tabu_swap_delta;
    double delta;
    double add_delta_max, drop_delta_max, swap_delta_max;
    double tabu_add_delta_max, tabu_drop_delta_max, tabu_swap_delta_max;
    int add_arr[MAXCAN], drop_arr[MAXCAN], swap_arr1[MAXCAN], swap_arr2[MAXCAN];
    int tabu_add_arr[MAXCAN], tabu_drop_arr[MAXCAN], tabu_swap_arr1[MAXCAN], tabu_swap_arr2[MAXCAN];
    int add_len, drop_len, swap_len;
    int tabu_add_len, tabu_drop_len, tabu_swap_len;
    int iter = 0;
    int non_improve = 0;

    int *local_best;
    local_best = new int[Ins.num_v];
    memcpy(local_best, sol, sizeof(int) * Ins.num_v);

    double localBest_totalProfit = TProfit;
    double localBest_totalWeight = TWeight;

    for(int i = 0; i < Ins.num_v; i++)
        Tabu_list[i] = 0;

    initialize_sup_arrays(sol);

//    cout<<" the local best solution is:"<<endl;
//    for(int i = 0; i < numItem; i++)
//        cout<<local_best[i]<<" ";
//    cout<<endl;
//    cout<<"the total profit = "<< localBest_totalProfit <<", the total weight = "<<localBest_totalWeight<<endl;

    //while((1.0*clock()-Start_time)/CLOCKS_PER_SEC < Time_limit)
    while(non_improve < Non_imp_iters)
    {
        //for add move, complexity: O(|E|), |E| is the set of incompatible items
        add_delta_max = -99999999;
        tabu_add_delta_max = -99999999;
        add_len = 0;
        tabu_add_len = 0;


        //for add move
        for(int i = 0; i < Ins.num_v; i++)
        {    //如果可以被选择加入
            if(sol[i] == 0)
            {
                delta = compute_flip_move_gain(i, sol);

                // if(Tabu_list[i] <= iter)
                {
                    if(delta > add_delta_max)
                    {
                        add_delta_max = delta;
                        add_len = 0;
                        add_arr[add_len++] = i;
                    }
                    else if(delta == add_delta_max && add_len < MAXCAN)
                        add_arr[add_len++] = i;
                }
                // else
                // {
                //     if(delta > tabu_add_delta_max)
                //     {
                //         tabu_add_delta_max = delta;
                //         tabu_add_len = 0;
                //         tabu_add_arr[tabu_add_len++] = i;
                //     }
                //     else if(delta == tabu_add_delta_max && tabu_add_len < MAXCAN)
                //         tabu_add_arr[tabu_add_len++] = i;
                // }
            }
        }


        //for drop move
        drop_delta_max=-99999999;
        tabu_drop_delta_max=-99999999;
        drop_len=0;
        tabu_drop_len=0;


        for(int i = 0; i < Ins.num_v; i++)
        {
            if (sol[i] == 1 && ((TWeight - Ins.weight[i]) > Ins.capacity + PRECISION
                                   || fabs(TWeight - Ins.weight[i] - Ins.capacity) <= PRECISION))
            {
                delta = compute_flip_move_gain(i, sol);;

                if(Tabu_list[i] <= iter)
                {
                    if(delta > drop_delta_max)
                    {
                        drop_delta_max = delta;
                        drop_len = 0;
                        drop_arr[drop_len++] = i;
                    }
                    else if(delta == drop_delta_max && drop_len < MAXNUM)
                        drop_arr[drop_len++] = i;
                }
                else
                {
                    if(delta > tabu_drop_delta_max)
                    {
                        tabu_drop_delta_max = delta;
                        tabu_drop_len = 0;
                        tabu_drop_arr[tabu_drop_len++] = i;
                    }
                    else if(delta == tabu_drop_delta_max && tabu_drop_len < MAXCAN)
                        tabu_drop_arr[tabu_drop_len++] = i;
                }
            }
        }


        //3. Swap
        swap_delta_max = -MAXVALUE;
        tabu_swap_delta_max = -MAXVALUE;
        swap_len = 0;
        tabu_swap_len = 0;

        int add_array_swap[500];
        int drop_array_swap[500];
        int add_len_swap = 0;
        int drop_len_swap = 0;
        for (int i = 0; i < Ins.num_v; i++)
        {
            //Find add nodes with max and sec Min_dis[i]
            if (sol[i] == 0 && (fabs(Min_dis[i] - Max_min_dis) <= PRECISION
                                   || fabs(Min_dis[i] - Sec_min_dis) <= PRECISION))
                add_array_swap[add_len_swap++] = i;
            //Find all drop nodes with a distance equal to sol.cost
            if (sol[i] == 1 && fabs(Min_dis[i] - TProfit) <= PRECISION)
                drop_array_swap[drop_len_swap++] = i;
        }

        for(int i = 0; i < add_len_swap; i++)
        {
            int nx = add_array_swap[i];
            for (int j = 0; j < drop_len_swap; j++)
            {
                int ny = drop_array_swap[j];
                if ((TWeight + Ins.weight[nx] - Ins.weight[ny]) < Ins.capacity - PRECISION)
                    continue;
                delta = compute_swap_move_gain(nx, ny, sol);

                //if(Tabu_list[i] <= iter && Tabu_list[j] <= iter)
                if(Tabu_list[ny] <= iter)
                {
                    if(delta>swap_delta_max)
                    {
                        swap_delta_max=delta;
                        swap_len=0;
                        swap_arr1[swap_len]=ny;
                        swap_arr2[swap_len]=nx;
                        swap_len++;
                    }
                    else if(delta==swap_delta_max && swap_len < MAXNUM)
                    {
                        swap_arr1[swap_len]=ny;
                        swap_arr2[swap_len]=nx;
                        swap_len++;
                    }
                }
                else
                {
                    if(delta>tabu_swap_delta_max)
                    {
                        tabu_swap_delta_max=delta;
                        tabu_swap_len=0;
                        tabu_swap_arr1[tabu_swap_len]=ny;
                        tabu_swap_arr2[tabu_swap_len]=nx;
                        tabu_swap_len++;

                    }
                    else if(delta==tabu_swap_delta_max && tabu_swap_len <MAXNUM)
                    {
                        tabu_swap_arr1[tabu_swap_len]=ny;
                        tabu_swap_arr2[tabu_swap_len]=nx;
                        tabu_swap_len++;
                    }
                }
            }
        }

        //select the best move from add, drop, swap neighborhoods
        int move_type = INFEA;
        double delta_maximum = -99999999;
        //cout<<"add_delta_max="<<add_delta_max<<", tabu_add_delta_max="<<tabu_add_delta_max<<", drop_delta_max="<<drop_delta_max<<", tabu_drop_delta_max"<<tabu_drop_delta_max
        //	<<", swap_delta_max="<<swap_delta_max<<", tabu_swap_delta_max="<<tabu_swap_delta_max<<endl;
        //cout<<" add_len="<<add_len<<", tabu_add_len="<<tabu_add_len<<", drop_len="<<drop_len<<", tabu_dorp_len="<<tabu_drop_len<<", swap_len=" <<swap_len<<", tabu_swap_len="<<tabu_swap_len<<endl;
        if (add_len > 0 && add_delta_max > delta_maximum)
        {
            move_type = 1;
            delta_maximum = add_delta_max;
        }
        if ((tabu_add_len > 0 && tabu_add_delta_max > delta_maximum && TProfit + tabu_add_delta_max > localBest_totalProfit))
        {
            move_type = 2;
            delta_maximum = tabu_add_delta_max;
        }
        if (drop_len > 0 && drop_delta_max > delta_maximum)
        {
            move_type = 3;
            delta_maximum = drop_delta_max;
        }
        if ((tabu_drop_len > 0 && tabu_drop_delta_max > delta_maximum && TProfit + tabu_drop_delta_max > localBest_totalProfit))
        {
            move_type = 4;
            delta_maximum = tabu_drop_delta_max;
        }
        if (swap_len > 0 && swap_delta_max > delta_maximum)
        {
            move_type = 5;
            delta_maximum = swap_delta_max;
        }
        if ((tabu_swap_len > 0 && tabu_swap_delta_max > delta_maximum && TProfit + tabu_swap_delta_max > localBest_totalProfit))
        {
            move_type = 6;
            delta_maximum = tabu_swap_delta_max;
        }


        int item1 = INFEA, item2 = INFEA, item = INFEA;
        int rx = INFEA;
        switch(move_type)
        {
            case 1:
                rx = rand() % add_len;
                item = add_arr[rx];
                break;
            case 2:
                rx = rand() % tabu_add_len;
                item = tabu_add_arr[rx];
                break;
            case 3:
                rx = rand() % drop_len;
                item = drop_arr[rx];
                break;
            case 4:
                rx = rand() % tabu_drop_len;
                item = tabu_drop_arr[rx];
                break;
            case 5:
                rx = rand() % swap_len;
                item1 = swap_arr1[rx];
                item2 = swap_arr2[rx];
                break;
            case 6:
                rx = rand() % tabu_swap_len;
                item1 = tabu_swap_arr1[rx];    //item1 : drop
                item2 = tabu_swap_arr2[rx];    //item2 : add
                break;
        }


        if (move_type == 1 || move_type == 2)
        {
            sol[item] = 1;
            TWeight += Ins.weight[item];
            //Tabu_list[item] = Tabu_tenure + iter;

        }
        else if (move_type == 3 || move_type == 4)
        {
            sol[item] = 0;
            TWeight -= Ins.weight[item];
            Tabu_list[item] = Tabu_tenure + iter;

        }
        else if (move_type == 5 || move_type == 6)
        {
            sol[item1] = 0;
            sol[item2] = 1;
            TWeight = TWeight + Ins.weight[item2] - Ins.weight[item1];
            Tabu_list[item1] = Tabu_tenure + iter;
            //Tabu_list[item2] = Tabu_tenure + iter;

        }
        if (move_type != INFEA)
        {
            TProfit += delta_maximum;
            f_sol = TProfit;
            if (TProfit > localBest_totalProfit)
            {
                non_improve = 0;
                memcpy(local_best, sol, sizeof(int) * Ins.num_v);
                localBest_totalProfit = TProfit;
            }
            else
                non_improve++;
        }
        iter++;
        //verify_solution(sol);

        if (item != -1 || item1 != -1 || item2 != -1)
        {
            initialize_sup_arrays(sol);
            //		verify_sol(sol); //verify solution
        }

        if (f_sol > Best_sol_one_run_obj + PRECISION)
            update_best_sol_one_run(localBest_totalProfit, local_best);
    }

    //copy local_best to current
    copy_solution(local_best, sol);
    //update global solution
    f_sol = localBest_totalProfit;

    build_data(sol, local_best);

    TProfit = localBest_totalProfit;

    delete[]local_best; local_best=NULL;

    return f_sol;
}


//knapsack-based crossover operator
void element_uniform_cross_over(int *offspring)
{

    int par1 = rand() % Pop_size;     //par1和par2是[0, pop_size]内的随机数
    int par2 = rand() % Pop_size;
    int *rand_arr = new int[Ins.num_v];
    while(par2 == par1)
        par2 = rand() % Pop_size;
    for(int i = 0; i < Ins.num_v; i++)
        offspring[i] = 0;
    TWeight = 0;
    TProfit = 0;
    num_sel = 0;
    for(int i = 0; i < Ins.num_v; i++)
        rand_arr[i] = i;

    for(int i = 0; i < Ins.num_v; i++)
    {
        int num = i + rand() % (Ins.num_v - i);		 // 取随机数
        int temp = rand_arr[i];
        rand_arr[i] = rand_arr[num];
        rand_arr[num] = temp;
    }

    int count = 0;
    while(count < Ins.num_v && TWeight < Ins.capacity - PRECISION)
    {
        int rx = rand() % 2;     //取随机数0，1
        int ver = rand_arr[count];
        if (rx == 0 && Pop_sol[par1][ver])
        {
            offspring[ver] = Pop_sol[par1][ver];
            TWeight += Ins.weight[ver];
            num_sel ++;
        }
        else if (rx == 1 && Pop_sol[par2][ver])
        {
            offspring[ver] = Pop_sol[par2][ver];
            TWeight += Ins.weight[ver];
            num_sel ++;
        }
        count ++;
    }

    if(TWeight < Ins.capacity - PRECISION)
    {
        int select = rand() % Ins.num_v;
        initialize_sup_arrays(offspring);
        //greedy expansion
        while (TWeight < Ins.capacity - PRECISION)
        {
            int element_best_array[10000];
            int best_len = 0;
            for (int i = 0; i < Ins.num_v; i++)
                if (offspring[i] == 0 && fabs(Min_dis[i] - Max_min_dis) <= PRECISION)
                    element_best_array[best_len++] = i;

            if (best_len > 0)
            {
                select = element_best_array[rand() % best_len];
                offspring[select] = 1;
                num_sel++;
                TWeight += Ins.weight[select];
                initialize_sup_arrays(offspring);
            }
        }
        TProfit = compute_obj(offspring);
        //	verify_sol(sol); //verify solution
    }
    delete []rand_arr; rand_arr = NULL;
}

void memetic()
{
    Best_sol_one_run_obj = -MAXNUM;
//    initial_hash();
    initial_population();

    int generations = 0;
    //while (1.0*(clock() - Start_time) / CLOCKS_PER_SEC < Time_limit)
    while(generations < Max_generations)
    {
//        element_knapsack_cross_over(Cur_sol);
        element_uniform_cross_over(Cur_sol);

//        cout<<"Cur_sol = "<<endl;
//        for(int i = 0; i < Ins.num_v; i++)
//            cout<<Cur_sol[i]<<" ";
//        cout<<endl;
//        double pp = compute_obj(Cur_sol);
//        cout<<"Cur_sol_obj = "<<pp<<endl;

        build_data(Cur_sol, Cur_sol);

        double f_sol = compute_obj(Cur_sol);

        double f_des = descent_local_search(Cur_sol, f_sol);

        // double f_ts = solution_based_tabu_search(Cur_sol, f_sol);
        double f_ts = multi_neighbor_tabu_search(Cur_sol, f_sol);



        // for(int i = 0; i < Ins.num_v; i++)
        //     cout<<Cur_sol[i]<<" ";
        // cout<<endl;

        update_population(Cur_sol, f_sol);
        generations++;
        cout << "generations=" << generations << ", Best_sol_one_run_obj=" << Best_sol_one_run_obj << ", time=" << 1.0*(clock() - Start_time) / CLOCKS_PER_SEC << endl;

    }
}
void MSBTS()
{
    Start_time = clock();
    Run_time = 0.0;

    int iter = 0;
    initial_hash();

    while(Run_time < Time_limit)    //MSBTS的停止条件是运行时间到达最大截止时间
    {
        memetic();

        Run_time = (clock() - Start_time) / CLOCKS_PER_SEC;

    }
}



int main(int argc, char ** argv)
{
    // 1. 检查参数个数：程序名本身算一个，后面带 3 个参数（实例路径，时间限制，随机种子），所以一共是 4 个
    if (argc < 4) {
        cout << "Usage: ./cdp_solver.exe <instance_path> <time_limit> <seed>" << endl;
        return -1;
    }

    string instanceName = argv[1];      // 接收实例路径
    Time_limit = stod(argv[2]);         // 接收时间限制（字符串转浮点数）
    seed = stoi(argv[3]);               // 接收随机种子（字符串转整数）

    srand(seed);

    // 2. 读取数据并分配内存
    read_instance(instanceName.c_str());
    allocate_memory();
    build_neighbors();

    // 3. 执行算法
    MSBTS();

    // 4. 将结果写入 result.json 供 Python 读取
    ofstream out("result.json");
    if (out.is_open()) {
        out << "{";
        out << "\"objective\": " << Best_sol_one_run_obj << ",";
        out << "\"elapsed_time\": " << Run_time << ",";
        out << "\"solution_vector\": [";
        for (int i = 0; i < Ins.num_v; i++) {
            out << Best_sol_one_run[i];
            if (i < Ins.num_v - 1) out << ",";
        }
        out << "]}";
        out.close();
        cout << "结果已写入 result.json" << endl;
    } else {
        cout << "无法写入 result.json" << endl;
    }

    free_memory();
    return 0;
}