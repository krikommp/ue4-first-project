# Learn Unread Engine 4

## Chapter 01 <u>Actor And Actor Components</u>

1. 属性

   `EditInstanceOnly` 在属性窗口中可见并且可编辑，在原型中不可见

   `VisibleInstanceOnly` 在属性窗口中可见，不可编辑

   `EditDefaultsOnly` 在属性窗口中不可见，在原型中可见可编辑

   `VisibleDefaultsOnly` 在属性窗口中不可见，在原型中可见不可编辑

   `EditAnywhere` 在任何地方都可见可编辑

2. Actor 基本操作

   `GetActorLocation()` 用于获取Actor的位置信息 返回 <u>FVector</u> 类型

   `SetActorLocation(FVector)` 用于设置Actor的位置

   `AddActorLocalOffset()` 增加Actor位置的偏移,。bSweep 参数为true时，不论Physics -> Simulate Physics 是否为true，都会被其他的Collision阻挡。这个方向是物体的局部坐标。

   `AddActorWorldOffset()` 与Local版本相同，增加的方向是世界坐标。     

3. Collision

   - Simpler Collision 简易的碰撞箱，一般由模型自带，没有的话默认不会生成
   - Complex Collision 可以由UE4自动计算出（根据顶点？） 但是会拖慢游戏速度

   - 要使用物理组件的话，需要先在Mesh上拥有Collision 然后打开 Physics -> Simulate Physics 

4. 旋转

   旋转是根据(pitch, Yaw, Roll),  Pitch是绕Y轴旋转，Yaw是绕Z轴旋转，Roll是绕X轴旋转。

   `AddActorLocalRotation()` 设置局部旋转量。

   `AddActorWorldRotation()` 设置世界旋转量。

5. Force

   是一个矢量，用FVector表示

   `StaticMesh->AddForce()` 对物体提供一个方向力

   `AddTorquelnRadians()`  对物体提供一个扭力

6. Random Numbers

   `FMath::FRand()` 返回(0,1)之间的值， 包含

   `FMath::FRandRange()` 返回区间值，包含

7. 删除C++文件

   需要关闭编辑器，关闭VS后在文件浏览器中删除对应文件及Binaries文件夹，然后再重新生成该项目。

## Chapter02 Pawn Class

需要通过GameMode来设置DefaultPawnClass

1. Pawn Movement Input

   -  打开Project Setting -> Input 设置输入映射
   - C++ Pawn class 中在`SetupPlayerInputComponent`中注册回调函数
   - `BindAxis` 注册Axis回调函数

2. SphereComponent

   一个简易的球形碰撞组件

## Chapter03 Gameplay

1. BoxComponent

   一个简易的盒形碰撞组件

   在制作一个游戏对象时，当这个游戏对象拥有碰撞组件，一般都将这个碰撞组件作为`RootComponent`

   > 设置`RootComponent` ： `RootComponent = Component`
   >
   > 获取`RootComponent` :	`GetRootComponent()`

   - 在代码中注册碰撞事件

     ```c++
     	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AFloorSwitch::OnOverlapBegin);	// 发生碰撞事件
     	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AFloorSwitch::OnOverlapEnd);		// 碰撞结束事件
     ```

     注册函数的参数可以通过进入`OnComponentBeginOverlap`中查看

   - 在代码中设置碰撞属性

     ```C++
     	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);		// 碰撞类型，只触发，不进行物理计算
     	TriggerBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
     	TriggerBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
     	TriggerBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
     ```

     > 可以先把所有的碰撞组全部设置为`Ignore`
     >
     > 然后再逐一设置需要检测的碰撞组

     上面的参数针对蓝图中Collision的信息

     ![Collision](.\image\collision_msg.png)

     > Collision Enable : 碰撞类型。只触发碰撞和是否进行物理计算
     >
     > Object Type: 针对哪中类型物体进行碰撞检测。

   - `UFUNCTION(BlueprintImplementableEvent, Category = "Floor Switch")`

     其中`BlueprintImplementableEvent` 可以让蓝图实现c++中的方法

     这个方法在蓝图中作为Event来使用

   - 在蓝图中实现控制物体的动画效果可以更加方便（借助Timeline）， 直接在代码中硬编码将会失去灵活性。

     > Timeline
     >
     > 双击可以进入Timeline设置界面
     >
     > 目前使用第一个Float, 按住Shift点击界面可以创建一个节点
     >
     > ![Timeline_vertical_horizontal](.\image\Timeline_vertical_horizontal.png)
     >
     > 点击上图按钮可以显示过大的节点（有些节点值比较大，可能超出屏幕，这时点击这两个按钮适配显示）
     >
     > 按住Shift同时点击设置的节点可以设置线性关系（线性/曲线）
     >
     > ![Timeline_Viewport](.\image\Timeline_Viewport.png)
     >
     > Play：开始Timeline
     >
     > Update：每帧调用函数
     >
     > New Time：Timeline时间，根据这个时间把值计算出来
     >
     > Reverse：反向计算值，并且会调用Update函数
     >
     > 勾选“User Last Keyframe”表示使用时间轴最后一个关键帧所在时间点作为结束时间，而不是使用设置的![Timeline_Use_Last_Keyframe](.\image\Timeline_Use_Last_Keyframe.png)5秒作为结束时间点。
   
   - `TSubclassOf<class ACritter>` 
   
     这个类型可以在蓝图中或者C++代码中指定一个Actor，用于实例化
   
   - 在c++中创建蓝图可以调用的函数
   
     ```C++
     UFUNCTION(BlueprintPure, Category = "Spawning")
     FVector GetSpawnPoint();
     ```
   
     `BlueprintPure` 创建一个具有返回值，但是没有Exec pin的函数
   
   - 获取BoxComponent长度
   
     `SpawningBox->GetScaledBoxExtent()`
   
   - 获取组件位置
   
     `SpawningBox->GetComponentLocation()`
   
   - 如何计算box内随机的一个点
   
     `UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent)`
   
     需要引入`Kismet/KismetMathLibrary.h`头文件
   
   - 允许c++函数可以在蓝图中重写
   
     ```C++
     UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spawning")
     void SpawnOurPawn(UClass* ToSpawn, const FVector& Location);
     ```
   
     如上图所示，使用关键词`BlueprintNativeEvent`来指定该函数可以被蓝图重写
   
     在cpp文件实现中需要对该函数加上`_Implementation`后缀
   
     ```C++
     void ASpawnVolume::SpawnOurPawn_Implementation(UClass* ToSpawn, const FVector& Location) {
     	if (ToSpawn) {
     		UWorld* world = GetWorld();
     		FActorSpawnParameters SpawnParam;
     		if (world) {
     			ACritter* CritterSpawned = world->SpawnActor<ACritter>(ToSpawn, Location, FRotator(0.f), SpawnParam);
     		}
     	}
     }
     ```
   
     在蓝图中可以直接调用该函数（类似于`BlueprintCallable`），如下图
   
     ![BlueprintNativeEvent_1](.\image\BlueprintNativeEvent_1.png)
   
     同时也会产生一个Event事件（类似于使用了`BlueprintImplementableEvent`），当这个上图函数被调用的时候就会触发这个事件，同时我们就可以根据这个事件来重写我们的c++函数，如下图所示。
   
     ![BlueprintNativeEvent_2](.\image\BlueprintNativeEvent_2.png)
   
     此时右键该Event，选择`Add call to parent function`可以使用蓝图父类即c++中的代码来重写函数（类似于Super::ParentFunction()）,如下图所示。
   
     ![BlueprintNativeEvent_3](.\image\BlueprintNativeEvent_3.png)
   
   - meta
   
     ```C++
     UPROPERTY(EditAnywhere, meta = (MakeEditWidget = "true"))
         FVector EndPoint;
     ```
   
     创建一个显示在场景中的gui，用来更方便的进行一些可视化配置，比如拖动配置一个结束坐标
   
   - 播放粒子效果的方法
   
     创建一个`class UParticleSystem* OverlapParticles;`对象
   
     `UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticles, GetActorLocation(), FRotator(0.f), true);`
   
     需要引入`Kismet/GameplayStatics.h`头文件
   
   - 播放声音的方法（2d）
   
     创建一个`class USoundCue* OverlapSound;`对象
   
     `UGameplayStatics::PlaySound2D(this, OverlapSound);`
   
   - 如何隐藏一个组件
   
     `Component->Deactivate()`
   
   - 在ue4中如果对一个空指针进行操作，将会引起引擎crash
   
     ```C++
     bRotate = false;
     Character->GetEquipWeapon()->Destroy();
     RightHandSocket->AttachActor(this, Character->GetMesh());
     Character->SetEquipWeapon(this);
     Character->SetActiveOverlapWeapon(nullptr);
     ```
   
     如上图，Destory的时候没有检查这个指针是否为空值。
   
   - 

