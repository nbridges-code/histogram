# histogram
This program creates a histogram of the word count above 6 characters in length of a text file. It uses a concurrent buffer that implement a producer-consumer pattern and operates with help from POSIX threads. This code has a focus on multi-threading. 

Example output is included below. Text files included are from Project Gutenburg (https://www.gutenberg.org/).

```
./histogram wap.txt

6 48559
7 40294
8 28249
9 16466
10 10122
11 4065
12 2573
13 1300
14 408
15 124
16 57
17 9
18 3
19 0
20 0
```


```
./histogram moby.txt
6 17275
7 14582
8 10072
9 6534
10 3587
11 1901
12 1064
13 571
14 180
15 71
16 22
17 12
18 1
19 0
20 1
21 0
22 0
23 0
24 0
25 0
```
