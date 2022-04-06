package main

import (
	"bufio"
	"fmt"
	"io"
	"log"
	"math"
	"os"
	"strconv"
	"strings"
	"sync"
)

func main() {
	var wg sync.WaitGroup
	// 	wg.Add(20)
	filePth := "../target_txs_id.txt"
	treePth := "../forward_id.txt"
	txPth := "../forward_tx_xx_id_info.jl"
	sz := 38663652
	const shard_count, shard_item_count, max_depth = 20, 6624, 30
	var target_ids [shard_count][shard_item_count]int32
	tree := make([][]int32, sz, sz)
	tx := make([][]float64, sz, sz)
	write_one_level_feature := func(fp *os.File, levels *[30000000]int32, cnt int) {
		tx_count := cnt
		coinbaseCount := 0
		// tx static - input
		var min, max, ave, std, sum [15]float64
		var overflow_id [15]int
		overflow_cnt := 0
		flag_init_min_max := false
		for ic := 0; ic < cnt; ic++ {
			t := tx[levels[ic]]
			if t == nil {
				coinbaseCount++
				continue
			}
			if !flag_init_min_max {
				for i := 0; i < 15; i++ {
					min[i] = t[i]
					max[i] = t[i]
				}
				flag_init_min_max = true
			}
			for i := 0; i < 15; i++ {
				sum[i] += t[i]
				if min[i] > t[i] {
					min[i] = t[i]
				}
				if max[i] < t[i] {
					max[i] = t[i]
				}
			}
		}
		if tx_count == coinbaseCount {
			_, err := fmt.Fprintf(fp, "%d\t%d", tx_count, coinbaseCount)
			if err != nil {
				log.Fatal(err)
			}
			_, err = fmt.Fprint(fp, "\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0")
			if err != nil {
				log.Fatal(err)
			}
			return
		}
		_, err := fmt.Fprintf(fp, "%d\t%d\t%d", tx_count, coinbaseCount, tx_count-coinbaseCount)
		if err != nil {
			log.Fatal(err)
		}
		for i := 0; i < 15; i++ {
			if sum[i] == math.Inf(1) {
				overflow_id[overflow_cnt] = i
				overflow_cnt++
			} else {
				ave[i] = sum[i] / float64(tx_count-coinbaseCount)
			}
		}
		if overflow_cnt > 0 {
			log.Println("overflow, tx_count:", tx_count)
			x_count := float64(tx_count - coinbaseCount)
			for ic := 0; ic < cnt; ic++ {
				t := tx[levels[ic]]
				if t == nil {
					continue
				}
				for i := 0; i < overflow_cnt; i++ {
					id := overflow_id[i]
					ave[id] += t[id] / x_count
				}
			}
		}
		for ic := 0; ic < cnt; ic++ {
			t := tx[levels[ic]]
			if t == nil {
				continue
			}
			for i := 0; i < 15; i++ {
				std[i] += math.Abs(t[i] - ave[i])
			}
		}
		for i := 0; i < 15; i++ {
			std[i] = math.Sqrt(std[i] / float64(tx_count-coinbaseCount))
		}
		for i := 0; i < 15; i++ {
			fmt.Fprintf(fp, "\t%f\t%f\t%f\t%f\t%f", sum[i], max[i], min[i], std[i], ave[i])
		}
	}
	write_feature := func(i int32, fp *os.File, level1 *[30000000]int32, level2 *[30000000]int32) {
		vised := [1208240]int{}
		level1[0] = i
		write_one_level_feature(fp, level1, 1)
		cnt_1 := 1
		cnt_2 := 0
		flag := 1
		for i := 0; i < max_depth; i++ {
			if flag == 1 {
				for j := 0; j < cnt_1; j++ {
					if tree[level1[j]] != nil {
						for _, nt := range tree[level1[j]] {
							idx := nt / 32
							res := nt % 32
							if vised[idx]&(1<<res) == 0 {
								level2[cnt_2] = nt
								cnt_2++
								vised[idx] |= 1 << res
							}
						}
					}
				}
				if cnt_2 == 0 {
					break
				}
				fmt.Fprint(fp, "\t")
				write_one_level_feature(fp, level2, cnt_2)
				flag = 2
				cnt_1 = 0
			} else {
				for j := 0; j < cnt_2; j++ {
					if tree[level2[j]] != nil {
						for _, nt := range tree[level2[j]] {
							idx := nt / 32
							res := nt % 32
							if vised[idx]&(1<<res) == 0 {
								level1[cnt_1] = nt
								cnt_1++
								vised[idx] |= 1 << res
							}
						}
					}
				}
				if cnt_1 == 0 {
					break
				}
				fmt.Fprint(fp, "\t")
				write_one_level_feature(fp, level1, cnt_1)
				flag = 1
				cnt_2 = 0
			}
		}
	}

	f, err := os.Open(filePth)
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()

	bfRd := bufio.NewReader(f)
	i := 0
	j := 0
	for {
		line, err := bfRd.ReadBytes('\n')

		if err != nil { //遇到任何错误立即返回，并忽略 EOF 错误信息
			if err == io.EOF {
				log.Println("Read finished", i, j)
				break
			}
			log.Fatal(err)
		}
		linestr := string(line[:len(line)-1])
		tid, err := strconv.Atoi(linestr)
		target_ids[i][j] = int32(tid)
		if err != nil {
			log.Fatal(err)
		}
		j++
		if j == 6624 {
			j = 0
			i++
		}
	}
	f, err = os.Open(treePth)
	if err != nil {
		log.Fatal(err)
	}
	bfRd = bufio.NewReader(f)
	ii := 0
	for {
		line, err := bfRd.ReadBytes('\n')
		if err != nil { //遇到任何错误立即返回，并忽略 EOF 错误信息
			if err == io.EOF {
				log.Println("Read finished", ii)
				break
			}
			log.Fatal(err)
		}
		if len(line) > 1 {
			linestr := string(line[1 : len(line)-2])
			next_ids := strings.Split(linestr, ", ")
			le := len(next_ids)
			ids := make([]int32, le)
			for i := 0; i < le; i++ {
				tmp, err := strconv.Atoi(next_ids[i])
				ids[i] = int32(tmp)
				if err != nil {
					log.Fatal(err)
				}
			}
			tree[ii] = ids
		}
		ii += 1
	}
	scanner := bufio.NewScanner(os.Stdin)
	log.Println("load_tree finished!")
	scanner.Scan()
    f, err = os.Open(txPth)
	if err != nil {
		log.Fatal(err)
	}
	ii = 0
	bfRd = bufio.NewReader(f)
	for {
		line, err := bfRd.ReadBytes('\n')
		if err != nil { //遇到任何错误立即返回，并忽略 EOF 错误信息
			if err == io.EOF {
				log.Println("Read finished", ii)
				break
			}
			log.Fatal(err)
		}
		if len(line) > 1 {
			linestr := string(line[1 : len(line)-2])
			vss := strings.Split(linestr, ", ")
			//var vs [15]float64
			vs := make([]float64, 15, 15)
			for i := 0; i < 15; i++ {
				vs[i], err = strconv.ParseFloat(vss[i], 64)
				if err != nil {
					log.Fatal(err)
				}
			}
			tx[ii] = vs
		}
		ii += 1
	}
    log.Println("load_tx finished!")
	scanner.Scan()
	for i := 0; i < 20; i++ {
		wg.Add(1)
		go func(i int) {
			fp, err := os.OpenFile(strconv.Itoa(i)+".log", os.O_WRONLY|os.O_CREATE, 0666)
			defer fp.Close()
			if err != nil {
				log.Fatal(err.Error())
			}
			var level1, level2 [30000000]int32
			for j := 0; j < shard_item_count; j++ {
//                 log.Println(i, j, "Start")
				_, err = fmt.Fprintf(fp, "%d\t", target_ids[i][j])
				if err != nil {
					log.Fatal(err)
				}
				write_feature(target_ids[i][j], fp, &level1, &level2)
				_, err = fmt.Fprint(fp, "\n")
				if err != nil {
					log.Fatal(err)
				}
				log.Println(i, j, "Finished")
			}
			log.Println(i, "Finished")
			wg.Done()
		}(i)
	}
	log.Println("Waiting to finish")
	wg.Wait()
	log.Println("\nTerminating Program")
}
