#include <iostream>
#include <thread>
#include <fstream>
#include <sstream>
#include <sys/time.h>
#include <memory.h>
#include <math.h>
using namespace std;
const int shard_count = 20, shard_item_count = 6624, max_depth = 30;
const int tx_count = 38663652;
const int tree_nt_count = 89048753;
const int tx_info_count = 38402729;
int target[shard_count][shard_item_count];
int target_len[shard_count];
// char target_file[] = "../target_txs_id.txt";
char target_file[] = "../target_negative_txs_id.txt";
char tree_file[] = "../forward_id.txt";
char tx_file[] = "../forward_tx_xx_id_info.jl";
int *tree_index_s;
int *tree_index_e;
int *tree_data;
typedef double d15[15];
d15 *tx;

long long getCurrentTime() {   
    struct timeval tv;    
    gettimeofday(&tv, NULL);    //该函数在sys/time.h头文件中
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;    
}

void write_one_level_feature(ofstream &fp, int *levels, int cnt, ofstream &logf) {
	int tx_count = cnt;
	int coinbaseCount = 0;
	// tx static - input
	double min[15] = {0}, max[15] = {0}, ave[15] = {0}, std[15] = {0}, sum[15] = {0};
	int overflow_id[15];
	int overflow_cnt = 0;
	bool flag_init_min_max = false;
	for (int ic = 0; ic < cnt; ic++) {
		double *t = tx[levels[ic]];
		if (t[0] < 0) {
			coinbaseCount++;
			continue;
		}
		if (!flag_init_min_max) {
			for (int i = 0; i < 15; i++) {
				min[i] = t[i];
				max[i] = t[i];
			}
			flag_init_min_max = true;
		}
		for (int i = 0; i < 15; i++) {
			sum[i] += t[i];
			if (min[i] > t[i]) {
				min[i] = t[i];
			}
			if (max[i] < t[i]) {
				max[i] = t[i];
			}
		}
	}
	if (tx_count == coinbaseCount) {
		fp << tx_count << '\t' << coinbaseCount;
		fp << "\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0";
		return;
	}
	fp << tx_count << '\t' << coinbaseCount << '\t' << (tx_count-coinbaseCount);
	for (int i = 0; i < 15; i++) {
		if (isinf(sum[i]) == 1) {
			overflow_id[overflow_cnt] = i;
			overflow_cnt++;
		} else {
			ave[i] = sum[i] / double(tx_count-coinbaseCount);
		}
	}
	if (overflow_cnt > 0) {
		logf << "overflow, tx_count:" << tx_count << endl;
		double x_count = double(tx_count - coinbaseCount);
		for (int ic = 0; ic < cnt; ic++) {
			double* t = tx[levels[ic]];
			if (t[0] < 0) {
				continue;
			}
			for (int i = 0; i < overflow_cnt; i++) {
				int id = overflow_id[i];
				ave[id] += t[id] / x_count;
			}
		}
	}
	for (int ic = 0; ic < cnt; ic++) {
		double* t = tx[levels[ic]];
		if (t[0] < 0) {
			continue;
		}
		for (int i = 0; i < 15; i++) {
			std[i] += fabs(t[i] - ave[i]);
		}
	}
	for (int i = 0; i < 15; i++) {
		std[i] = sqrt(std[i] / double(tx_count-coinbaseCount));
	}
	for (int i = 0; i < 15; i++) {
//         cout << i << '\t' << sum[i] << '\t' << &sum[i] << ' ' << max[i] << '\t' << min[i] << '\t' << std[i] << '\t' << ave[i] << endl;
		fp << '\t' << sum[i] << '\t' << max[i] << '\t' << min[i] << '\t' << std[i] << '\t' << ave[i];
	}
}


void task(int sid) {
	char ofname[] = "a.txt";
	ofname[0] += sid;
	ofstream out_f(ofname);
	ofname[2] = 'l'; 
	ofname[3] = 'o';
	ofname[4] = 'g';
	ofstream log_f(ofname);
    out_f.precision(400);
	int *vised = new int[1208240];
	int *level1 = new int [30000000], *level2 = new int [30000000];
	for (int i = 0; i < shard_item_count; i++) {
//         cout << "task" << i << endl;
		log_f << sid << '\t' << i << "\tStarted\t" << getCurrentTime() << endl;
        out_f << target[sid][i] << '\t';
		memset(vised, 0, sizeof(int)*1208240);
		level1[0] = target[sid][i];
//         cout << "level" << 1 << "\ttx count" << 1 << endl;
		write_one_level_feature(out_f, level1, 1, log_f);
		int cnt_1 = 1;
		int cnt_2 = 0;
		int flag = 1;
		for (int j = 0; j < max_depth; j++) {
			if (flag == 1) {
				for (int k = 0; k < cnt_1; k++) {
					int cid = level1[k];
					for (int l = tree_index_s[cid]; l < tree_index_e[cid]; l++) {
						int nt = tree_data[l];
						int idx = nt / 32;
						int res = nt % 32;
						if ((vised[idx]&(1<<res)) == 0) {
							level2[cnt_2++] = nt;
							vised[idx] |= 1 << res;
						}
					}
				}
				if (cnt_2 == 0) {
					break;
				}
				out_f << '\t';
//                 cout << "level" << j+1 << "\ttx count" << cnt_2 << endl;
				write_one_level_feature(out_f, level2, cnt_2, log_f);
				flag = 2;
				cnt_1 = 0;
			} else {
				for (int k = 0; k < cnt_2; k++) {
					int cid = level2[k];
					for (int l = tree_index_s[cid]; l < tree_index_e[cid]; l++) {
						int nt = tree_data[l];
						int idx = nt / 32;
						int res = nt % 32;
						if ((vised[idx]&(1<<res)) == 0) {
							level1[cnt_1++] = nt;
							vised[idx] |= 1 << res;
						}
					}
				}
				if (cnt_1 == 0) {
					break;
				}
				out_f << '\t';
//                 cout << "level" << j+1 << "\ttx count" << cnt_1 << endl;
				write_one_level_feature(out_f, level1, cnt_1, log_f);
				flag = 1;
				cnt_2 = 0;
			}
		}
		log_f << sid << '\t' << i << "\tFinished\t" << getCurrentTime() << endl;
		out_f << '\n';
	}
	delete []vised;
	delete []level1;
	delete []level2;
}

int main()
{
	// Load target id 
	ifstream target_f(target_file);
	for (int i = 0; i < shard_count; i ++) {
		for (int j = 0; j < shard_item_count; j++) {
			if (!(target_f >> target[i][j]))
                break;
			target_len[i] = j + 1;
		}
        cout << i << ' ' << target_len[i] << ' ' << target[i][0] << ' '  << target[i][target_len[i]-1] << endl;
	}
	cout << "Target loaded " << getCurrentTime() << endl;
	// Load tree structure
	tree_index_s = new int[tx_count];
	tree_index_e = new int[tx_count];
	tree_data = new int[tree_nt_count];
	ifstream tree_f(tree_file);
	string temp;
	stringstream sstream;
	int index = 0, did = 0;
	char tempch;
	while (getline(tree_f, temp)) {
		tree_index_s[index] = did;
		if (temp.length() > 1) {
			sstream << temp;
			int t;
			sstream >> tempch; // [
			while (sstream >> t) {
				tree_data[did++] = t;
				sstream >> tempch; // ,
			}
			sstream.clear();
		}
		tree_index_e[index] = did;
		index ++;
	}
	cout << "Tree loaded " << getCurrentTime() << endl;
// 	cin >> tempch;
	// Load tx info
	tx = new d15[tx_count];
	ifstream tx_f(tx_file);
	index = 0;
	while (getline(tx_f, temp)) {
		if (temp.length() > 1) {
			sstream << temp;
			sstream >> tempch; // [
			for (int i = 0; i < 15; i++) {
				sstream >> tx[index][i];
				sstream >> tempch; // ,
			}
			sstream.clear();
		} else {
			tx[index][0] = -1;
		}
		index ++;
	}
	cout << "Tx loaded " << getCurrentTime() << endl;
// 	cin >> temp;
	// Start threads
    cout << "Start" << endl;
//     task(0);
//     return 0;
	thread *threads[shard_count];
	for (int i = 0; i < shard_count; i++) {
		threads[i] = new thread(task, i);
	}
    for (int i = 0; i < shard_count; i++) {
		threads[i]->join();
		delete threads[i];
	}
	delete []tree_index_s;
	delete []tree_index_e;
	delete []tree_data;
	delete []tx;
    cout << "Finished" << endl;
    return 0;
}
