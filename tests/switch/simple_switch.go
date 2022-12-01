package main

func main() {

    size := "XXXL"

    switch size {

    case "XXS":
        println("очень очень маленький")
        break
    case "XXL":
        println("очень очень большой")
        break
    default:
        println("неизвестно")
    }
}