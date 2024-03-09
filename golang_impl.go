package main

import (
	"crypto/sha1"
	"fmt"
	"sort"
	"sync"
	"time"
)

func timingDecorator(f func() string) func() string {
	return func() string {
		startTime := time.Now()
		result := f()
		elapsedTime := time.Since(startTime).Seconds()
		fmt.Printf("Execution time: %.4f seconds\n", elapsedTime)
		return result
	}
}

func sha1hash(threadCounts int, inputValue string) string {
	subHashes := calculateHash(threadCounts, inputValue)
	return mergeHashes(subHashes)
}

func mergeHashes(resultQueue chan string) string {
	close(resultQueue)
	var result string
	var resultObjs []string
	for item := range resultQueue {
		resultObjs = append(resultObjs, item)
	}
	sort.Strings(resultObjs)
	for _, item := range resultObjs {
		result += item
	}
	return result
}

func calculateHash(k int, inputValue string) chan string {
	var wg sync.WaitGroup
	resultQueue := make(chan string, k)

	for i := 0; i < k; i++ {
		wg.Add(1)
		go newHashCalculator(i, inputValue, resultQueue, &wg)
	}

	wg.Wait()
	return resultQueue
}

func newHashCalculator(threadID int, inputValue string, resultQ chan string, wg *sync.WaitGroup) {
	defer wg.Done()

	counter := 0
	sha1Object := sha1.New()
	lastSHA1 := ""

	for counter < 1<<20 {
		result := fmt.Sprintf("%d%s%d%s", threadID, lastSHA1, counter, inputValue)
		sha1Object.Write([]byte(result))
		lastSHA1 = fmt.Sprintf("%x", sha1Object.Sum(nil))
		sha1Object.Reset()
		counter++
	}

	resultQ <- lastSHA1
}

func main() {
	sha1Result := timingDecorator(func() string {
		return sha1hash(5, "ya ali")
	})

	fmt.Println(sha1Result())
}
