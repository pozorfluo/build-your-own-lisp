# quench your curiosity like this :
python -m timeit -s 'import random' 'random.randrange(1)'
python -m timeit -s 'import random' 'random.getrandbits(1)'
python -m timeit -s 'import random' 'int(random.random())'
python -m timeit -s 'import random' 'random.randrange(16)'
python -m timeit -s 'import random' 'random.getrandbits(4)'
python -m timeit -s 'import random' 'int(random.random() * 16)'
python -m timeit -s 'import random; things = [i for i in range(16)]' 'int(random.random() * len(things))'
python -m timeit -s 'import random; things = [i for i in range(16)]' 'random.choice(things)'
python -m timeit -s 'import random; things = [i for i in range(16)]' 'things[int(random.random() * len(things))]'
python -m timeit -s 'import random; things = [i for i in range(16)]; count = len(things)' 'things[random.getrandbits(4)]'
python -m timeit -s 'import random; things = [i for i in range(16)]; count = len(things) - 1' 'things[min(count,random.getrandbits(5))]'
python -m timeit -s 'import random; things = [i for i in range(16)]; count = len(things) - 1' 'things[random.getrandbits(5) % count]'
python -m timeit -s 'import random; things = [i for i in range(16)]; count = len(things)' 'things[int(random.random() * count)]'
python -m timeit -s 'import random; things = [i for i in range(16)]; count = len(things); rnd = lambda : things[int(random.random() * count)]' 'rnd()'

python -m timeit -s 'import random' 'random.randint(0, 255)'
python -m timeit -s 'import random' 'int(random.random() * 256)'

python -m timeit -s 'import random; bounds = random.getrandbits(8)' 'random.random() * bounds - (bounds * random.getrandbits(1))'
python -m timeit -s 'import random; bounds = random.getrandbits(8)' 'random.uniform(-bounds, bounds)'
python -m timeit -s 'import random; bounds = random.getrandbits(8)' 'random.triangular(-bounds, bounds)'
python -m timeit -s 'import random' 'random.sample(range(10000000), k=60)'



# mini = sys.float_info.max
# for x in range(10000000):
# 	bounds = random.getrandbits(32) 
# 	# new_r = random.random() * bounds - (bounds * random.getrandbits(1))
# 	new_r = random.uniform(-bounds, bounds)
# 	mini = min(mini, abs(new_r))

# print(mini)