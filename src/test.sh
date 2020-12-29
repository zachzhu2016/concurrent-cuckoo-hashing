# 1. trials 2. cap 3. loadfactor 4. n_threads 5. n_operations

# n_threads
./obj64/hashset 100 100 95 8 400 >  output
./obj64/hashset 100 100 95 4 400 >> output
./obj64/hashset 100 100 95 2 400 >> output
./obj64/hashset 100 100 95 1 400 >> output

# loadfactor
./obj64/hashset 100 100 60 8 400 >> output
./obj64/hashset 100 100 70 8 400 >> output
./obj64/hashset 100 100 80 8 400 >> output
./obj64/hashset 100 100 90 8 400 >> output

# n_operations 
./obj64/hashset 100 100 95 8 200 >> output
./obj64/hashset 100 100 95 8 400 >> output
./obj64/hashset 100 100 95 8 600 >> output
./obj64/hashset 100 100 95 8 800 >> output

