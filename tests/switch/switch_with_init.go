package main

func main() {
    switch num := 6; num % 2 == 0 {
    case true:
        println("even value")
        fallthrough
    case false:
        println("odd value")
    }
}