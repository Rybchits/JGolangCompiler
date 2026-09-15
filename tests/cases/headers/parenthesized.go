package main
type T struct { X int }
func main() {
    if (T{X: 1}).X == 1 { println(1) }
    for (T{X: 1}).X == 1 { break }
    switch (T{X: 1}).X { case 1: println(1); }
}
