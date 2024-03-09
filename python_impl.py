import threading
from hashlib import sha1
from queue import Queue
from time import time


def timing_decorator(func):
    def wrapper(*args, **kwargs):
        start_time = time()
        result = func(*args, **kwargs)
        end_time = time()
        elapsed_time = end_time - start_time
        print(f"Execution time: {elapsed_time:.4f} seconds")
        return result
    return wrapper


@timing_decorator
def sha1hash(thread_counts: int, input_value: str):
    sub_hashes = calculate_hash(thread_counts, input_value)
    return merge_hashes(sub_hashes)


def merge_hashes(result_queue):
    result = ''
    result_objs = list(result_queue.queue)
    result_objs.sort()
    for item in result_objs:
        result += item[1]
    return result


def calculate_hash(k: int, input_value):
    thread_pool = []
    result_queue = Queue()
    for i in range(k):
        worker_thread = threading.Thread(
            target=new_hash_calculator, args=(i,input_value, result_queue,))
        thread_pool.append(worker_thread)
    for thread in thread_pool:
        thread.start()
    for thread in thread_pool:
        thread.join()

    return result_queue


def new_hash_calculator(thread_id,input_value, result_q):
    # retults=f"{thread_id}{last_SHA1}{counter}{input}"
    counter = 0
    sha1_object = sha1()
    last_SHA1 = ''

    while counter < 2**20:
        result = f"{threading.get_ident()}{last_SHA1}{counter}{input_value}"
        sha1_object.update(result.encode("utf-8"))
        last_SHA1 = sha1_object.hexdigest()
        counter += 1
    result_q.put((thread_id, last_SHA1))


if __name__ == '__main__':
    sha1 = sha1hash(1, "ya ali")
    print(sha1)
