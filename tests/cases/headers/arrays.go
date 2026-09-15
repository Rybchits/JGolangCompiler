package main
func main() {
    if []int{1}[0] == 1 { println(1) }
    if [1]int{1}[0] == 1 { println(1) }
    for _, row := range [][]int{{1}, {2}} { println(row) }
}
