package main
type T struct { X int }
func main() {
    flags := []bool{true}
    if flags[T{X: 0}.X] { println(1) }
    for flags[T{X: 0}.X] { break }
    switch flags[T{X: 0}.X] { case true: println(1); }
}
