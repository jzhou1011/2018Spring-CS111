
#Run your program with a single thread, and increasing numbers of iterations (10, 100, 1000, 10000, 20000)
./lab2_list --threads=1 --iterations=10 >>lab2_list.csv
./lab2_list --threads=1 --iterations=100 >>lab2_list.csv
./lab2_list --threads=1 --iterations=1000 >>lab2_list.csv
./lab2_list --threads=1 --iterations=10000 >>lab2_list.csv
./lab2_list --threads=1 --iterations=20000 >>lab2_list.csv
#Run your program and see how many parallel threads (2,4,8,12) and iterations (1, 10,100,1000) it takes to fairly consistently demonstrate a problem.
./lab2_list --threads=2 --iterations=1 >>lab2_list.csv
./lab2_list --threads=2 --iterations=10 >>lab2_list.csv
./lab2_list --threads=2 --iterations=100 >>lab2_list.csv
./lab2_list --threads=2 --iterations=1000 >>lab2_list.csv
./lab2_list --threads=4 --iterations=1 >>lab2_list.csv
./lab2_list --threads=4 --iterations=10 >>lab2_list.csv
./lab2_list --threads=4 --iterations=100 >>lab2_list.csv
./lab2_list --threads=4 --iterations=1000 >>lab2_list.csv
./lab2_list --threads=8 --iterations=1 >>lab2_list.csv
./lab2_list --threads=8 --iterations=10 >>lab2_list.csv
./lab2_list --threads=8 --iterations=100 >>lab2_list.csv
./lab2_list --threads=8 --iterations=1000 >>lab2_list.csv
./lab2_list --threads=12 --iterations=1 >>lab2_list.csv
./lab2_list --threads=12 --iterations=10 >>lab2_list.csv
./lab2_list --threads=12 --iterations=100 >>lab2_list.csv
./lab2_list --threads=12 --iterations=1000 >>lab2_list.csv
#Then run it again using various combinations of yield options and 
#see how many threads (2,4,8,12) and iterations (1, 2,4,8,16,32) it takes.
#--yield=i
./lab2_list --threads=2 --iterations=1 --yield=i >>lab2_list.csv
./lab2_list --threads=2 --iterations=2 --yield=i >>lab2_list.csv
./lab2_list --threads=2 --iterations=4 --yield=i >>lab2_list.csv
./lab2_list --threads=2 --iterations=8 --yield=i >>lab2_list.csv
./lab2_list --threads=2 --iterations=16 --yield=i >>lab2_list.csv
./lab2_list --threads=2 --iterations=32 --yield=i >>lab2_list.csv
./lab2_list --threads=4 --iterations=1 --yield=i >>lab2_list.csv
./lab2_list --threads=4 --iterations=2 --yield=i >>lab2_list.csv
./lab2_list --threads=4 --iterations=4 --yield=i >>lab2_list.csv
./lab2_list --threads=4 --iterations=8 --yield=i >>lab2_list.csv
./lab2_list --threads=4 --iterations=16 --yield=i >>lab2_list.csv
./lab2_list --threads=4 --iterations=32 --yield=i >>lab2_list.csv
./lab2_list --threads=8 --iterations=1 --yield=i >>lab2_list.csv
./lab2_list --threads=8 --iterations=2 --yield=i >>lab2_list.csv
./lab2_list --threads=8 --iterations=4 --yield=i >>lab2_list.csv
./lab2_list --threads=8 --iterations=8 --yield=i >>lab2_list.csv
./lab2_list --threads=8 --iterations=16 --yield=i >>lab2_list.csv
./lab2_list --threads=8 --iterations=32 --yield=i >>lab2_list.csv
./lab2_list --threads=12 --iterations=1 --yield=i >>lab2_list.csv
./lab2_list --threads=12 --iterations=2 --yield=i >>lab2_list.csv
./lab2_list --threads=12 --iterations=4 --yield=i >>lab2_list.csv
./lab2_list --threads=12 --iterations=8 --yield=i >>lab2_list.csv
./lab2_list --threads=12 --iterations=16 --yield=i >>lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=i >>lab2_list.csv
#--yield=d
./lab2_list --threads=2 --iterations=1 --yield=d >>lab2_list.csv
./lab2_list --threads=2 --iterations=2 --yield=d >>lab2_list.csv
./lab2_list --threads=2 --iterations=4 --yield=d >>lab2_list.csv
./lab2_list --threads=2 --iterations=8 --yield=d >>lab2_list.csv
./lab2_list --threads=2 --iterations=16 --yield=d >>lab2_list.csv
./lab2_list --threads=2 --iterations=32 --yield=d >>lab2_list.csv
./lab2_list --threads=4 --iterations=1 --yield=d >>lab2_list.csv
./lab2_list --threads=4 --iterations=2 --yield=d >>lab2_list.csv
./lab2_list --threads=4 --iterations=4 --yield=d >>lab2_list.csv
./lab2_list --threads=4 --iterations=8 --yield=d >>lab2_list.csv
./lab2_list --threads=4 --iterations=16 --yield=d >>lab2_list.csv
./lab2_list --threads=4 --iterations=32 --yield=d >>lab2_list.csv
./lab2_list --threads=8 --iterations=1 --yield=d >>lab2_list.csv
./lab2_list --threads=8 --iterations=2 --yield=d >>lab2_list.csv
./lab2_list --threads=8 --iterations=4 --yield=d >>lab2_list.csv
./lab2_list --threads=8 --iterations=8 --yield=d >>lab2_list.csv
./lab2_list --threads=8 --iterations=16 --yield=d >>lab2_list.csv
./lab2_list --threads=8 --iterations=32 --yield=d >>lab2_list.csv
./lab2_list --threads=12 --iterations=1 --yield=d >>lab2_list.csv
./lab2_list --threads=12 --iterations=2 --yield=d >>lab2_list.csv
./lab2_list --threads=12 --iterations=4 --yield=d >>lab2_list.csv
./lab2_list --threads=12 --iterations=8 --yield=d >>lab2_list.csv
./lab2_list --threads=12 --iterations=16 --yield=d >>lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=d >>lab2_list.csv
#--yield=il
./lab2_list --threads=2 --iterations=1 --yield=il >>lab2_list.csv
./lab2_list --threads=2 --iterations=2 --yield=il >>lab2_list.csv
./lab2_list --threads=2 --iterations=4 --yield=il >>lab2_list.csv
./lab2_list --threads=2 --iterations=8 --yield=il >>lab2_list.csv
./lab2_list --threads=2 --iterations=16 --yield=il >>lab2_list.csv
./lab2_list --threads=2 --iterations=32 --yield=il >>lab2_list.csv
./lab2_list --threads=4 --iterations=1 --yield=il >>lab2_list.csv
./lab2_list --threads=4 --iterations=2 --yield=il >>lab2_list.csv
./lab2_list --threads=4 --iterations=4 --yield=il >>lab2_list.csv
./lab2_list --threads=4 --iterations=8 --yield=il >>lab2_list.csv
./lab2_list --threads=4 --iterations=16 --yield=il >>lab2_list.csv
./lab2_list --threads=4 --iterations=32 --yield=il >>lab2_list.csv
./lab2_list --threads=8 --iterations=1 --yield=il >>lab2_list.csv
./lab2_list --threads=8 --iterations=2 --yield=il >>lab2_list.csv
./lab2_list --threads=8 --iterations=4 --yield=il >>lab2_list.csv
./lab2_list --threads=8 --iterations=8 --yield=il >>lab2_list.csv
./lab2_list --threads=8 --iterations=16 --yield=il >>lab2_list.csv
./lab2_list --threads=8 --iterations=32 --yield=il >>lab2_list.csv
./lab2_list --threads=12 --iterations=1 --yield=il >>lab2_list.csv
./lab2_list --threads=12 --iterations=2 --yield=il >>lab2_list.csv
./lab2_list --threads=12 --iterations=4 --yield=il >>lab2_list.csv
./lab2_list --threads=12 --iterations=8 --yield=il >>lab2_list.csv
./lab2_list --threads=12 --iterations=16 --yield=il >>lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=il >>lab2_list.csv
#--yield=dl
./lab2_list --threads=2 --iterations=1 --yield=dl >>lab2_list.csv
./lab2_list --threads=2 --iterations=2 --yield=dl >>lab2_list.csv
./lab2_list --threads=2 --iterations=4 --yield=dl >>lab2_list.csv
./lab2_list --threads=2 --iterations=8 --yield=dl >>lab2_list.csv
./lab2_list --threads=2 --iterations=16 --yield=dl >>lab2_list.csv
./lab2_list --threads=2 --iterations=32 --yield=dl >>lab2_list.csv
./lab2_list --threads=4 --iterations=1 --yield=dl >>lab2_list.csv
./lab2_list --threads=4 --iterations=2 --yield=dl >>lab2_list.csv
./lab2_list --threads=4 --iterations=4 --yield=dl >>lab2_list.csv
./lab2_list --threads=4 --iterations=8 --yield=dl >>lab2_list.csv
./lab2_list --threads=4 --iterations=16 --yield=dl >>lab2_list.csv
./lab2_list --threads=4 --iterations=32 --yield=dl >>lab2_list.csv
./lab2_list --threads=8 --iterations=1 --yield=dl >>lab2_list.csv
./lab2_list --threads=8 --iterations=2 --yield=dl >>lab2_list.csv
./lab2_list --threads=8 --iterations=4 --yield=dl >>lab2_list.csv
./lab2_list --threads=8 --iterations=8 --yield=dl >>lab2_list.csv
./lab2_list --threads=8 --iterations=16 --yield=dl >>lab2_list.csv
./lab2_list --threads=8 --iterations=32 --yield=dl >>lab2_list.csv
./lab2_list --threads=12 --iterations=1 --yield=dl >>lab2_list.csv
./lab2_list --threads=12 --iterations=2 --yield=dl >>lab2_list.csv
./lab2_list --threads=12 --iterations=4 --yield=dl >>lab2_list.csv
./lab2_list --threads=12 --iterations=8 --yield=dl >>lab2_list.csv
./lab2_list --threads=12 --iterations=16 --yield=dl >>lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=dl >>lab2_list.csv
#eliminate all of the problems, even for moderate numbers of threads (12) and iterations (32)
#mutex
#i option
./lab2_list --threads=12 --iterations=1 --yield=i --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=2 --yield=i --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=4 --yield=i --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=8 --yield=i --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=16 --yield=i --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=i --sync=m >>lab2_list.csv
#d option
./lab2_list --threads=12 --iterations=1 --yield=d --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=2 --yield=d --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=4 --yield=d --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=8 --yield=d --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=16 --yield=d --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=d --sync=m >>lab2_list.csv
#il option
./lab2_list --threads=12 --iterations=1 --yield=il --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=2 --yield=il --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=4 --yield=il --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=8 --yield=il --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=16 --yield=il --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=il --sync=m >>lab2_list.csv
#dl option
./lab2_list --threads=12 --iterations=1 --yield=dl --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=2 --yield=dl --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=4 --yield=dl --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=8 --yield=dl --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=16 --yield=dl --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=dl --sync=m >>lab2_list.csv
#spin lock
#i option
./lab2_list --threads=12 --iterations=1 --yield=i --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=2 --yield=i --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=4 --yield=i --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=8 --yield=i --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=16 --yield=i --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=i --sync=s >>lab2_list.csv
#d option
./lab2_list --threads=12 --iterations=1 --yield=d --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=2 --yield=d --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=4 --yield=d --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=8 --yield=d --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=16 --yield=d --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=d --sync=s >>lab2_list.csv
#il option
./lab2_list --threads=12 --iterations=1 --yield=il --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=2 --yield=il --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=4 --yield=il --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=8 --yield=il --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=16 --yield=il --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=il --sync=s >>lab2_list.csv
#dl option
./lab2_list --threads=12 --iterations=1 --yield=dl --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=2 --yield=dl --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=4 --yield=dl --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=8 --yield=dl --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=16 --yield=dl --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=dl --sync=s >>lab2_list.csv
#Choose an appropriate number of iterations (e.g. 1000) to overcome start-up costs and 
#rerun your program without the yields for a range of # threads (1, 2, 4, 8, 12, 16, 24)
./lab2_list --threads=2 --iterations=1000 >>lab2_list.csv
./lab2_list --threads=4 --iterations=1000 >>lab2_list.csv
./lab2_list --threads=8 --iterations=1000 >>lab2_list.csv
./lab2_list --threads=12 --iterations=1000 >>lab2_list.csv
./lab2_list --threads=16 --iterations=1000 >>lab2_list.csv
./lab2_list --threads=24 --iterations=1000 >>lab2_list.csv
./lab2_list --threads=1 --iterations=1000 --sync=s >>lab2_list.csv
./lab2_list --threads=2 --iterations=1000 --sync=s >>lab2_list.csv
./lab2_list --threads=4 --iterations=1000 --sync=s >>lab2_list.csv
./lab2_list --threads=8 --iterations=1000 --sync=s >>lab2_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=s >>lab2_list.csv
./lab2_list --threads=16 --iterations=1000 --sync=s >>lab2_list.csv
./lab2_list --threads=24 --iterations=1000 --sync=s >>lab2_list.csv
./lab2_list --threads=1 --iterations=1000 --sync=m >>lab2_list.csv
./lab2_list --threads=2 --iterations=1000 --sync=m >>lab2_list.csv
./lab2_list --threads=4 --iterations=1000 --sync=m >>lab2_list.csv
./lab2_list --threads=8 --iterations=1000 --sync=m >>lab2_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=m >>lab2_list.csv
./lab2_list --threads=16 --iterations=1000 --sync=m >>lab2_list.csv
./lab2_list --threads=24 --iterations=1000 --sync=m >>lab2_list.csv
