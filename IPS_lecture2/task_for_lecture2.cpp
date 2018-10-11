#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>
#include <chrono>
#include <iostream>

using namespace std::chrono;

/// Функция ReducerMaxTest() определяет максимальный элемент массива,
/// переданного ей в качестве аргумента, и его позицию
/// mass_pointer - указатель исходный массив целых чисел
/// size - количество элементов в массиве
void ReducerMaxTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_max_index<long, int>> maximum;
	cilk_for(long i = 0; i < size; ++i)
	{
		maximum->calc_max(i, mass_pointer[i]);
	}
	printf("Maximal element = %d has index = %d\n\n",
		maximum->get_reference(), maximum->get_index_reference());
}

/// Функция ReducerMinTest() определяет минимальный элемент массива,
/// переданного ей в качестве аргумента, и его позицию
/// mass_pointer - указатель исходный массив целых чисел
/// size - количество элементов в массиве
void ReducerMinTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_min_index<long, int>> minimum;
	cilk_for(long i = 0; i < size; ++i)
	{
		minimum->calc_min(i, mass_pointer[i]);
	}
	printf("Minimal element = %d has index = %d\n\n",
		minimum->get_reference(), minimum->get_index_reference());
}

/// Функция ParallelSort() сортирует массив в порядке возрастания
/// begin - указатель на первый элемент исходного массива
/// end - указатель на последний элемент исходного массива
/// time - флаг вывода времени
void ParallelSort(int *begin, int *end, bool time)
{
	high_resolution_clock::time_point t1, t2;

	if (time)
		t1 = high_resolution_clock::now();

	if (begin != end) 
	{
		--end;
		int *middle = std::partition(begin, end, std::bind2nd(std::less<int>(), *end));
		std::swap(*end, *middle); 
		cilk_spawn ParallelSort(begin, middle, false);
		ParallelSort(++middle, ++end, false);
		cilk_sync;
	}

	if (time)
	{
		t2 = high_resolution_clock::now();

		duration<double> duration = (t2 - t1);
		std::cout << "Elapsed time: " << duration.count() << " seconds" << std::endl << std::endl;
	}
}

/// Функция CompareForAndCilk_For() сортирует массив в порядке возрастания
/// sz - размер заполняемого массива
void CompareForAndCilk_For(size_t sz)
{
	high_resolution_clock::time_point t1, t2;

	std::vector<int> vec;
	cilk::reducer<cilk::op_vector<int>> red_vec;

	t1 = high_resolution_clock::now();

	for (size_t i = 0; i < sz; i++)
		vec.push_back(rand() % 20000 + 1);

	t2 = high_resolution_clock::now();

	duration<double> duration = (t2 - t1);
	std::cout << "Elapsed time with standart for: " << duration.count() << " seconds" << std::endl;

	t1 = high_resolution_clock::now();

	cilk_for (size_t i = 0; i < sz; i++)
		red_vec->push_back(rand() % 20000 + 1);

	t2 = high_resolution_clock::now();

	duration = (t2 - t1);
	std::cout << "Elapsed time with cilk_for: " << duration.count() << " seconds" << std::endl;
}


int main()
{
	srand((unsigned)time(0));

	// устанавливаем количество работающих потоков = 6
	__cilkrts_set_param("nworkers", "6");

	long i;
	const long mass_size = 5000000;
	int *mass_begin, *mass_end;
	int *mass = new int[mass_size]; 
	int *mass1 = new int[mass_size / 2];
	int *mass2 = new int[mass_size / 10];

	for(i = 0; i < mass_size; ++i)
	{
		mass[i] = (rand() % 25000) + 1;
	}
	for (i = 0; i < mass_size / 2; ++i)
	{
		mass1[i] = (rand() % 25000) + 1;
	}
	for (i = 0; i < mass_size / 10; ++i)
	{
		mass2[i] = (rand() % 25000) + 1;
	}
	
	mass_begin = mass;
	mass_end = mass_begin + mass_size;
	ReducerMaxTest(mass, mass_size);
	ReducerMinTest(mass, mass_size);

	std::cout << "ParalleSort - Size: " << mass_size / 10 << "; ";
	ParallelSort(mass2, mass2 + mass_size / 10, true);

	std::cout << "ParalleSort - Size: " << mass_size / 2 << "; ";
	ParallelSort(mass1, mass1 + mass_size / 2, true);

	std::cout << "ParalleSort - Size: " << mass_size << "; ";
	ParallelSort(mass_begin, mass_end, true);

	ReducerMaxTest(mass, mass_size);
	ReducerMinTest(mass, mass_size);

	std::cout << std::endl;

	// vector filling
	int sizes[8] = {1000000, 100000, 10000, 1000, 500, 100, 50, 10};
	for (i = 0; i < 8; i++)
	{
		std::cout << "Filling vector - Size: " << sizes[i] << std::endl;
		CompareForAndCilk_For(sizes[i]);
		std::cout << std::endl;
	}

	getchar();
	delete[] mass;
	return 0;
}