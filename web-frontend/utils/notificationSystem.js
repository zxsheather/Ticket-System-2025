/**
 * 通知系统
 * 处理实时通知和消息推送
 */

const logger = require('./logger');

class NotificationSystem {
  constructor() {
    this.notifications = new Map();
    this.userSockets = new Map();
    this.adminSockets = new Set();
  }

  /**
   * 初始化Socket.IO
   */
  initialize(io) {
    this.io = io;
    
    io.on('connection', (socket) => {
      this.handleConnection(socket);
    });
    
    logger.info('通知系统已初始化');
  }

  /**
   * 处理新连接
   */
  handleConnection(socket) {
    logger.info(`新连接: ${socket.id}`);

    // 用户认证
    socket.on('user_auth', (userData) => {
      this.authenticateUser(socket, userData);
    });

    // 加入房间
    socket.on('join_room', (room) => {
      socket.join(room);
      logger.info(`用户 ${socket.id} 加入房间: ${room}`);
    });

    // 离开房间
    socket.on('leave_room', (room) => {
      socket.leave(room);
      logger.info(`用户 ${socket.id} 离开房间: ${room}`);
    });

    // 标记通知为已读
    socket.on('mark_notification_read', (notificationId) => {
      this.markNotificationAsRead(socket.username, notificationId);
    });

    // 获取未读通知
    socket.on('get_unread_notifications', () => {
      if (socket.username) {
        const notifications = this.getUserNotifications(socket.username);
        socket.emit('unread_notifications', notifications);
      }
    });

    // 管理员事件
    socket.on('admin_action', (data) => {
      if (socket.isAdmin) {
        this.handleAdminAction(socket, data);
      }
    });

    // 断开连接
    socket.on('disconnect', () => {
      this.handleDisconnection(socket);
    });
  }

  /**
   * 用户认证
   */
  authenticateUser(socket, userData) {
    if (userData && userData.username) {
      socket.username = userData.username;
      socket.privilege = userData.privilege || 0;
      socket.isAdmin = userData.privilege >= 10;
      
      // 存储用户Socket
      this.userSockets.set(userData.username, socket);
      
      // 如果是管理员，加入管理员房间
      if (socket.isAdmin) {
        socket.join('admin');
        this.adminSockets.add(socket);
        logger.info(`管理员 ${userData.username} 已连接`);
      }
      
      // 发送欢迎消息
      socket.emit('auth_success', {
        message: '连接成功',
        isAdmin: socket.isAdmin
      });
      
      // 发送未读通知
      const notifications = this.getUserNotifications(userData.username);
      if (notifications.length > 0) {
        socket.emit('unread_notifications', notifications);
      }
      
      logger.info(`用户 ${userData.username} 已认证 (权限: ${userData.privilege})`);
    } else {
      socket.emit('auth_error', { message: '认证失败' });
    }
  }

  /**
   * 处理断开连接
   */
  handleDisconnection(socket) {
    if (socket.username) {
      this.userSockets.delete(socket.username);
      logger.info(`用户 ${socket.username} 已断开连接`);
    }
    
    if (socket.isAdmin) {
      this.adminSockets.delete(socket);
      logger.info(`管理员 ${socket.username} 已断开连接`);
    }
    
    logger.info(`连接 ${socket.id} 已断开`);
  }

  /**
   * 发送通知给特定用户
   */
  sendNotificationToUser(username, notification) {
    // 添加时间戳和ID
    const fullNotification = {
      id: this.generateNotificationId(),
      timestamp: Date.now(),
      read: false,
      ...notification
    };

    // 存储通知
    if (!this.notifications.has(username)) {
      this.notifications.set(username, []);
    }
    this.notifications.get(username).push(fullNotification);

    // 限制通知数量（最多保留100条）
    const userNotifications = this.notifications.get(username);
    if (userNotifications.length > 100) {
      userNotifications.splice(0, userNotifications.length - 100);
    }

    // 如果用户在线，实时发送
    const userSocket = this.userSockets.get(username);
    if (userSocket) {
      userSocket.emit('new_notification', fullNotification);
    }

    logger.info(`发送通知给用户 ${username}: ${notification.title}`);
    return fullNotification.id;
  }

  /**
   * 发送通知给所有用户
   */
  broadcastNotification(notification) {
    const results = [];
    this.userSockets.forEach((socket, username) => {
      const notificationId = this.sendNotificationToUser(username, notification);
      results.push({ username, notificationId });
    });
    
    logger.info(`广播通知: ${notification.title} (发送给 ${results.length} 个用户)`);
    return results;
  }

  /**
   * 发送通知给管理员
   */
  sendNotificationToAdmins(notification) {
    const fullNotification = {
      id: this.generateNotificationId(),
      timestamp: Date.now(),
      ...notification
    };

    this.adminSockets.forEach(socket => {
      socket.emit('admin_notification', fullNotification);
    });

    this.io.to('admin').emit('admin_broadcast', fullNotification);
    
    logger.info(`发送管理员通知: ${notification.title}`);
    return fullNotification.id;
  }

  /**
   * 获取用户通知
   */
  getUserNotifications(username, limit = 50) {
    const userNotifications = this.notifications.get(username) || [];
    return userNotifications
      .sort((a, b) => b.timestamp - a.timestamp)
      .slice(0, limit);
  }

  /**
   * 获取用户未读通知数量
   */
  getUnreadCount(username) {
    const userNotifications = this.notifications.get(username) || [];
    return userNotifications.filter(n => !n.read).length;
  }

  /**
   * 标记通知为已读
   */
  markNotificationAsRead(username, notificationId) {
    const userNotifications = this.notifications.get(username);
    if (userNotifications) {
      const notification = userNotifications.find(n => n.id === notificationId);
      if (notification) {
        notification.read = true;
        logger.info(`通知 ${notificationId} 已标记为已读 (用户: ${username})`);
        return true;
      }
    }
    return false;
  }

  /**
   * 标记所有通知为已读
   */
  markAllNotificationsAsRead(username) {
    const userNotifications = this.notifications.get(username);
    if (userNotifications) {
      userNotifications.forEach(notification => {
        notification.read = true;
      });
      logger.info(`用户 ${username} 的所有通知已标记为已读`);
      return true;
    }
    return false;
  }

  /**
   * 删除通知
   */
  deleteNotification(username, notificationId) {
    const userNotifications = this.notifications.get(username);
    if (userNotifications) {
      const index = userNotifications.findIndex(n => n.id === notificationId);
      if (index !== -1) {
        userNotifications.splice(index, 1);
        logger.info(`删除通知 ${notificationId} (用户: ${username})`);
        return true;
      }
    }
    return false;
  }

  /**
   * 处理管理员操作
   */
  handleAdminAction(socket, data) {
    const { action, target, params } = data;
    
    switch (action) {
      case 'broadcast_message':
        this.broadcastNotification({
          type: 'system',
          title: '系统通知',
          message: params.message,
          level: params.level || 'info'
        });
        break;
        
      case 'send_user_message':
        this.sendNotificationToUser(target, {
          type: 'admin',
          title: '管理员消息',
          message: params.message,
          level: params.level || 'info'
        });
        break;
        
      case 'system_maintenance':
        this.broadcastNotification({
          type: 'maintenance',
          title: '系统维护通知',
          message: params.message,
          level: 'warning',
          maintenanceTime: params.time
        });
        break;
        
      default:
        logger.warn(`未知的管理员操作: ${action}`);
    }
  }

  /**
   * 发送订单状态更新通知
   */
  sendOrderNotification(username, orderData) {
    const notification = {
      type: 'order',
      title: '订单状态更新',
      message: `您的订单 ${orderData.orderId} 状态已更新为: ${orderData.status}`,
      level: 'info',
      orderData: orderData
    };
    
    return this.sendNotificationToUser(username, notification);
  }

  /**
   * 发送车票购买成功通知
   */
  sendTicketPurchaseNotification(username, ticketData) {
    const notification = {
      type: 'ticket',
      title: '购票成功',
      message: `恭喜您成功购买了 ${ticketData.trainId} 次列车的车票`,
      level: 'success',
      ticketData: ticketData
    };
    
    return this.sendNotificationToUser(username, notification);
  }

  /**
   * 发送系统公告
   */
  sendSystemAnnouncement(title, message, level = 'info') {
    const notification = {
      type: 'announcement',
      title: title,
      message: message,
      level: level
    };
    
    return this.broadcastNotification(notification);
  }

  /**
   * 生成通知ID
   */
  generateNotificationId() {
    return Date.now().toString(36) + Math.random().toString(36).substr(2, 9);
  }

  /**
   * 清理过期通知
   */
  cleanupOldNotifications(maxAge = 7 * 24 * 60 * 60 * 1000) { // 默认7天
    const now = Date.now();
    let cleanedCount = 0;
    
    this.notifications.forEach((notifications, username) => {
      const oldLength = notifications.length;
      this.notifications.set(username, 
        notifications.filter(n => (now - n.timestamp) < maxAge)
      );
      cleanedCount += oldLength - this.notifications.get(username).length;
    });
    
    if (cleanedCount > 0) {
      logger.info(`清理了 ${cleanedCount} 条过期通知`);
    }
  }

  /**
   * 获取系统统计信息
   */
  getSystemStats() {
    let totalNotifications = 0;
    let totalUnread = 0;
    
    this.notifications.forEach(notifications => {
      totalNotifications += notifications.length;
      totalUnread += notifications.filter(n => !n.read).length;
    });
    
    return {
      totalUsers: this.notifications.size,
      totalNotifications: totalNotifications,
      totalUnread: totalUnread,
      onlineUsers: this.userSockets.size,
      onlineAdmins: this.adminSockets.size
    };
  }
}

module.exports = new NotificationSystem();
