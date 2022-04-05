# Introduction

This dataset contains 264,960 transactions between block 270,000 (2013-11-16 22:09:06 UTC) and block 300,000 (2014-05-10 06:32:34 UTC), labeled by Blockstream.info’s method of CoinJoin detection and a method of SharedCoin detection. In this period SharedCoin is active.

In order to provide more relevant information, the dataset includes a large number of transactions related to the above 264,960 transactions, but this transactions are not labeled.

There are a total of 116,537,811 transactions in the dataset, all of which are real Bitcoin transactions, but the original TXIDs have been replaced with consecutive integers starting from 0 to 116,537,810.

We have computed 15 features for each transaction and provided the ids of the precursor and successor transactions for these transactions.

# Download Data

[Google Drive](https://drive.google.com/drive/folders/1srpyBEXbaDhLg5juEQh-I71IxUA3JYx1?usp=sharing)

[Baidu Netdisk](https://pan.baidu.com/s/1CVTijokH1jr1uXx0tS2qBg)  (Access Code: `wnr5`)

## Tree Data


| File             | Size   | Lines       | Description                                      | Format                                             |
| ---------------- | ------ | ----------- | ------------------------------------------------ | -------------------------------------------------- |
| positive_ids.txt | 1.2 MB | 132,480     | IDs of positive cases (Coin Mixing Transactions) | One ID per line                                    |
| negative_ids.txt | 1.2 MB | 132,480     | IDs of negative cases                            | One ID per line                                    |
| tx_features.txt  | 14 GB  | 116,537,811 | Processed features of transactions               | One JSON List per line, empty line for Coinbase TX |
| tree_forward.txt | 4.6 GB | 116,537,811 | Precursor  of each node IDs                      | One JSON List per line, empty line for Coinbase TX |
| tree_back.txt    | 2.2 GB | 116,537,811 | Successor of each node in IDs                    | One JSON List per line, empty line for Unspent TX  |

There are 116,537,811 transactions in this dataset. The original TXID is removed and a consecutive integer starting from 0 is used as ID.

In `tx_features.txt`, `tree_forward.txt` and `tree_back.txt`, the first line refers to the Transaction of ID 0, the second line refers to the ID 1 ....



### Load Data

1. tree_forward.txt

   ```python
   import json
   
   forward_tree = []
   with open('tree_forward.txt') as f:
   	for l in f:
   		if l == '\n':
   			n = []
           else:
               n = json.loads(l)
           forward_tree.append(n)
   ```

   

2. tree_back.txt

   ```python
   import json
   
   backward_tree = []
   with open('tree_back.txt') as f:
   	for l in f:
   		if l == '\n':
   			n = []
           else:
               n = json.loads(l)
           backward_tree.append(n)
   ```

   

3. tx_features.txt

   ```python
   import json
   
   tx_features = []
   with open('tx_features.txt') as f:
   	for l in f:
   		if l == '\n':
   			n = []
           else:
               n = json.loads(l)
           tx_features.append(n)
   ```


## Processed Data

| File             | Size   | Lines       | Description                                      | Format                                             |
| ---------------- | ------ | ----------- | ------------------------------------------------ | -------------------------------------------------- |
| dev_set_full.zip | 1.43 GB | 79,488     |  Dev set                                   | One JSON per line |
| train_set_full.zip | 3.34 GB | 185,472     | Train set                            | One JSON per line                                    |


# Reference

Our [paper](https://link.springer.com/article/10.1007/s10489-021-02453-9) published in Applied Intelligence.

```
@article{sun2021lstm,
  title={LSTM-TC: Bitcoin coin mixing detection method with a high recall},
  author={Sun, Xiaowen and Yang, Tan and Hu, Bo},
  journal={Applied Intelligence},
  pages={1--14},
  year={2021},
  publisher={Springer}
}
```

