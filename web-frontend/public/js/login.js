// 用户登录页面JavaScript

(function() {
    'use strict';

    // DOM元素
    const loginForm = document.getElementById('loginForm');
    const submitButton = loginForm.querySelector('button[type="submit"]');
    const usernameField = document.getElementById('username');
    const passwordField = document.getElementById('password');

    // 表单验证函数
    function validateForm() {
        let isValid = true;

        // 验证用户名
        if (!usernameField.value.trim()) {
            showFieldError(usernameField, '请输入用户名');
            isValid = false;
        } else {
            hideFieldError(usernameField);
        }

        // 验证密码
        if (!passwordField.value) {
            showFieldError(passwordField, '请输入密码');
            isValid = false;
        } else {
            hideFieldError(passwordField);
        }

        return isValid;
    }

    // 显示字段错误
    function showFieldError(field, message) {
        field.classList.add('is-invalid');
        const feedback = field.parentNode.querySelector('.invalid-feedback');
        if (feedback) {
            feedback.textContent = message;
        }
    }

    // 隐藏字段错误
    function hideFieldError(field) {
        field.classList.remove('is-invalid');
    }

    // 显示全局消息
    function showMessage(message, type = 'success') {
        // 移除现有的消息
        const existingAlert = loginForm.querySelector('.alert');
        if (existingAlert) {
            existingAlert.remove();
        }

        // 创建新的消息
        const alertClass = type === 'success' ? 'alert-success' : 'alert-danger';
        const iconClass = type === 'success' ? 'fa-check-circle' : 'fa-exclamation-circle';
        
        const alertDiv = document.createElement('div');
        alertDiv.className = `alert ${alertClass} alert-dismissible fade show`;
        alertDiv.innerHTML = `
            <i class="fas ${iconClass}"></i> ${message}
            <button type="button" class="btn-close" data-bs-dismiss="alert"></button>
        `;

        // 插入到表单开头
        loginForm.insertBefore(alertDiv, loginForm.firstChild);

        // 滚动到消息位置
        alertDiv.scrollIntoView({ behavior: 'smooth', block: 'center' });
    }

    // 设置提交按钮状态
    function setSubmitButtonState(loading = false) {
        if (loading) {
            submitButton.disabled = true;
            submitButton.innerHTML = '<i class="fas fa-spinner fa-spin"></i> 登录中...';
        } else {
            submitButton.disabled = false;
            submitButton.innerHTML = '<i class="fas fa-sign-in-alt"></i> 登录';
        }
    }

    // 表单提交处理
    loginForm.addEventListener('submit', async function(e) {
        e.preventDefault();

        // 验证表单
        if (!validateForm()) {
            showMessage('请检查输入信息', 'error');
            return;
        }

        try {
            setSubmitButtonState(true);

            // 准备表单数据
            const formData = new FormData(loginForm);
            const data = Object.fromEntries(formData.entries());

            // 发送登录请求
            const response = await fetch('/api/user/login', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(data)
            });

            const result = await response.json();

            if (result.success) {
                showMessage('登录成功！正在跳转...', 'success');
                
                // 延迟跳转，让用户看到成功消息
                setTimeout(() => {
                    window.location.href = '/';
                }, 1000);
            } else {
                showMessage(result.message || '登录失败，请检查用户名和密码', 'error');
            }
        } catch (error) {
            console.error('登录错误:', error);
            showMessage('网络错误，请检查网络连接后重试', 'error');
        } finally {
            setSubmitButtonState(false);
        }
    });

    // 用户名输入验证
    usernameField.addEventListener('input', function() {
        if (this.value.trim()) {
            hideFieldError(this);
        }
    });

    // 密码输入验证
    passwordField.addEventListener('input', function() {
        if (this.value) {
            hideFieldError(this);
        }
    });

    // 页面加载完成后的处理
    document.addEventListener('DOMContentLoaded', function() {
        // 检查URL参数中的消息
        const urlParams = new URLSearchParams(window.location.search);
        const message = urlParams.get('message');
        const error = urlParams.get('error');
        
        if (message) {
            showMessage(decodeURIComponent(message), 'success');
            // 清理URL参数
            const newUrl = window.location.pathname;
            window.history.replaceState({}, document.title, newUrl);
        } else if (error) {
            showMessage(decodeURIComponent(error), 'error');
            // 清理URL参数
            const newUrl = window.location.pathname;
            window.history.replaceState({}, document.title, newUrl);
        }

        // 聚焦到第一个输入框
        usernameField.focus();
    });

    // 回车键快捷登录
    document.addEventListener('keypress', function(e) {
        if (e.key === 'Enter' && !submitButton.disabled) {
            loginForm.dispatchEvent(new Event('submit'));
        }
    });

})();
