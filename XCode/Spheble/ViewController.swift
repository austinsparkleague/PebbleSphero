//
//  ViewController.swift
//  Spheble
//
//  Created by Liam on June 26, 2016
//

import UIKit

enum AppMessageKey : Int {
    case keyButtonUp = 0
    case keyButtonDown
}

class ViewController: UIViewController, PBPebbleCentralDelegate {
    
    var pebbleCentral: PBPebbleCentral!
    var activeWatch: PBWatch?
    
    var robot : RKConvenienceRobot?
    
    var hasPresentedPleaseConnectSpheroMessage = false
     
    var colorCounter = 0
    var colors : [(Float, Float, Float)] = [(148.0/255.0, 0.0, 211.0/255.0), (75.0/255.0, 0.0, 130.0/255.0), (0, 0, 255.0/255.0), (0, 255.0/255.0, 0), (255.0/255.0, 255.0/255.0, 0), (255.0/255.0, 127.0/255.0, 0), (255.0/255.0, 0 , 0)]
    
    var newestX = 0;
    var newestY = 0;
    
    var isCalibrating = false;
    
    @IBOutlet weak var spheroLabel: UILabel!
    @IBOutlet weak var statusLabel: UILabel!
    var myAppUUID = UUID(uuidString: "3f90347a-a84a-4098-88dc-29adf7b291ba")
    
    
    var watch : PBWatch?
    var central : PBPebbleCentral?
    
    override func viewDidLoad() {
        
        super.viewDidLoad()
        
        // Pebble setup
        self.central = PBPebbleCentral.default()
        self.central?.delegate = self
        
        if let appUUID = myAppUUID {
            self.central?.appUUID = appUUID
        } else {
            print("UUID was not successfully created")
        }
        
        self.central?.run()
        
        //Sphero Setup
        RKRobotDiscoveryAgent.startDiscovery()
        
        RKRobotDiscoveryAgent.shared().addNotificationObserver(self, selector: #selector(handleRobotStateChangeNotification(_:)))
        
    }
    
    @IBAction func disconnectRobot(_ sender: AnyObject) {
        self.robot?.sleep()
    }
    
    func handleRobotStateChangeNotification(_ n: RKRobotChangedStateNotification) {
        switch n.type {
        case RKRobotChangedStateNotificationType.online:
            
            if let convRobot = n.robot {
                self.robot = RKConvenienceRobot(robot: convRobot)
                self.spheroLabel.text = "Connected to Sphero"
            } else {
                self.spheroLabel.text = "Could not convert robot"
            }
            
            break
            
        case RKRobotChangedStateNotificationType.disconnected:
            self.robot = nil
            self.spheroLabel.text = "Disconnected"
            break
        case RKRobotChangedStateNotificationType.failedConnect:
            break
        default:
            break
        }
        
    }
    
    
    func pebbleCentral(_ central: PBPebbleCentral, watchDidDisconnect watch: PBWatch) {
        if(self.watch == watch) {
            self.watch = nil
        }
    }
    
    func moveRobot() {
        
        // make sure we dont divide by zero :(
        if(self.newestX == 0) {
            self.newestX = 1
        }
        
        
        print(self.newestY)
        print(self.newestX)
        
        // Math Sucks
        var heading = atan(Float(Float(self.newestY)/Float(self.newestX))) * Float((180/M_PI))
        if(self.newestX < 0 && self.newestY > 0) {
            heading += 180
        } else if(self.newestX < 0 && self.newestY < 0) {
            heading += 180
        } else if(self.newestX > 0 && self.newestY < 0) {
            heading += 360
        }
        
        heading = Float(floor(heading))
        
        if (heading == 360) {
            heading = 0
        }
        
        let velocity = Float(max(abs(Float(self.newestY)), Float(abs(self.newestX))) / 800)
        
        print(String(heading) + " " + String(velocity))
        
        if (self.isCalibrating) {
            self.robot?.send(RKRollCommand(heading: heading, velocity: 0))
        } else {
            self.robot?.send(RKRollCommand(heading: heading, velocity: velocity))
        }
        
        
    }
    
    func pebbleCentral(_ central: PBPebbleCentral, watchDidConnect watch: PBWatch, isNew: Bool) {
        
        statusLabel.text = "Hello " + watch.name
        
        if(self.watch == watch) {
            return
        }
        self.watch = watch
        
        self.watch?.sportsAppLaunch({ (watch, error) in
            print("Launching app...")
            
        })
        
        self.watch?.appMessagesAddReceiveUpdateHandler({ (watch, info: [NSNumber : AnyObject]) -> Bool in
            
            if (self.robot == nil && self.hasPresentedPleaseConnectSpheroMessage == false) {
                
                let alertController = UIAlertController(title: "Warning!", message: "Please connect your sphero before launching the Pebble app", preferredStyle: UIAlertControllerStyle.alert)
                
                let OKAction = UIAlertAction(title: "OK", style: UIAlertActionStyle.default) { (action) in
                    // ...
                }
                alertController.addAction(OKAction)
                
                self.present(alertController, animated: true) {
                    // ...
                }
                
                self.hasPresentedPleaseConnectSpheroMessage = true

            }
            
            //self.statusLabel.text = "Recieved Message"
            
            if(info[0] != nil) {
                
                if(info[0] as! Int > 10000) {
                    self.newestY = info[0] as! Int - 4294967294
                } else {
                    self.newestY = info[0] as! Int
                }
                
                self.moveRobot()
                
            } else if(info[1] != nil) {
                
                if(info[1] as! Int > 10000) {
                    self.newestX = info[1] as! Int - 4294967294
                    
                } else {
                    self.newestX = info[1] as! Int
                }
                
                self.moveRobot()
                
            } else if (info[2] != nil) {
                self.isCalibrating = !self.isCalibrating
                if(self.isCalibrating) {
                    self.robot?.send(RKBackLEDOutputCommand(brightness: 1.0))
                } else {
                    self.robot?.setZeroHeading()
                    self.robot?.send(RKBackLEDOutputCommand(brightness: 0.0))
                }
            } else if (info[3] != nil) {
                
                self.statusLabel.text = "Cycled Color"
                
                let currentColor = self.colors[self.colorCounter % self.colors.count]
                self.colorCounter += 1
                
                self.robot?.send(RKRGBLEDOutputCommand(red: currentColor.0, green: currentColor.1, blue: currentColor.2))
                
            }
            
            return true
        })
        
        
    }
    
}
