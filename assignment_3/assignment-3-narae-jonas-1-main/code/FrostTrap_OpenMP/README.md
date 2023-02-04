
With a 300x300 array for FrostTrap: 

```
20:10:02-johnm@jmbhost:~/inf-3201/git/staff-repo/assignments/current/FrostTrap-TSP-pre\>./FrostTrap
Using solver simple with max-threads 8
Total iterations: 30417
{ 'host': 'jmbhost', 'usecs': 14029448, 'secs' : 14.029448, 'scheme' : 'simple', 'max_threads' : 8, 'total_iters' : 30417}
20:11:20-johnm@jmbhost:~/inf-3201/git/staff-repo/assignments/current/FrostTrap-TSP-pre\>python3 FrostTrap_numpy_arr_slices.py 
Creating trap.
Got trap of dimension (300, 300) and epsilon 90.0. Now iterating.
Computing took 30.69315767288208 seconds with final diff 89.9999419258658
```

The C code is single threaded and doesn't actually use OpenMP. The
Python version uses numpy arrays, but doesn't use any optimisations in
the way the arrays are used. Python+numpy is still within a factor 2 of C. 

