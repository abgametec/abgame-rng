# abgame-rng

## 说明

游戏为传统的百家乐玩法，总共为8副牌，总牌数为52*8=416张牌（除大小王），随机算法由C++写成，每次由当前的机器时间戳作为随机数种子`srand(time(0))`，采用C++函数`rand()`执行随机，保证了洗牌的纯随机规则。

说明如下：

```C++
        // 随机种子生成
        srand(time(0));

        // 生成所有8*52张牌
        for(int i=0; i<8; ++i){
            for(int j=1; j<=52; ++j){
                table.cards[i*52+j-1] = j;
            }
        }
        
        // 洗牌，依赖rand()函数打乱生成好的牌位置
        for(int i=0; i<52*8-1; ++i){
            int j = i+rand()%(52*8-i);
            std::swap(table.cards[i], table.cards[j]);
        }
```

C++ rand和srand函数说明

rand [https://en.cppreference.com/w/cpp/numeric/random/rand](https://en.cppreference.com/w/cpp/numeric/random/rand)

srand [https://en.cppreference.com/w/cpp/numeric/random/srand](https://en.cppreference.com/w/cpp/numeric/random/srand)
