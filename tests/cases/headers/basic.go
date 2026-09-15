package main
func main() {
    flag := true
    if flag { println(1) } else { println(0) }
    if n := 1; n > 0 { println(n) }
    for flag { break }
    for i := 0; i < 1; i++ { println(i) }
    for { break }
    switch flag { case true: println(1); }
}
