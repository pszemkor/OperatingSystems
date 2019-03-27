#! /bin/bash

num=500

ind1=0
ind2=1
ind3=2

./lab1 $"create_table" "$num" $"search_directory" $"/" $"tmp" $"tmp1.txt" $"search_directory" $"/usr" $"tmp" $"tmp2.txt" $"search_directory" $"/home" $"tmp" $"tmp3.txt" 2>>/dev/null

./lab1 $"create_table" "$num" $"add" $"tmp1.txt" $"add" $"tmp2.txt" $"add" $"tmp3.txt" $"remove_block" "$ind1" $"remove_block" "$ind2" $"remove_block" "$ind3" 2>>/dev/null

./lab1 $"create_table" "$num" $"add" $"tmp1.txt" $"remove_block" "$ind1" $"add" $"tmp2.txt" $"remove_block" "$ind2" $"add" $"tmp3.txt" $"remove_block" "$ind3" 2>>/dev/null