package main

import "fmt"

// Match the compiler's scalar output on stdout instead of Go's built-in stderr.
func print(value interface{}) { fmt.Print(value) }
func println(value interface{}) { fmt.Println(value) }

func readInt() int {
    var value int
    if _, err := fmt.Scan(&value); err != nil { panic(err) }
    return value
}
